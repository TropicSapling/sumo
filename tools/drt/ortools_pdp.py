# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2021-2022 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    ortools_pdp.py
# @author  Philip Ritzer
# @date    2021-12-16

"""
Capacitated vehicle routing problem with pickup and delivery.

Based on https://developers.google.com/optimization/routing/pickup_delivery#complete_programs
"""

from __future__ import print_function

import numpy as np

from ortools.constraint_solver import routing_enums_pb2
from ortools.constraint_solver import pywrapcp


def get_solution(data, manager, routing, solution, verbose):
    if verbose:
        print('Objective: ', solution.ObjectiveValue())
    solution_dict = {}
    total_distance = 0
    for vehicle_id in range(data['num_vehicles']):
        route = []
        index = routing.Start(vehicle_id)
        if verbose:
            plan_output = 'Route for vehicle %s:\n' % vehicle_id
        route_distance = 0
        while not routing.IsEnd(index):
            if verbose:
                plan_output += ' %s -> ' % manager.IndexToNode(index)
            route.append(manager.IndexToNode(index))
            previous_index = index
            index = solution.Value(routing.NextVar(index))
            route_distance += routing.GetArcCostForVehicle(
                previous_index, index, vehicle_id)
        route.append(manager.IndexToNode(index))
        if verbose:
            plan_output += '%s\n' % manager.IndexToNode(index)
            plan_output += 'Costs of the route: %sm\n' % route_distance
            print(plan_output)
        total_distance += route_distance
        solution_dict[vehicle_id] = [route, total_distance]
    return solution_dict


def main(data, time_limit_seconds=10, verbose=False):
    """Entry point of the program."""
    # Create the routing index manager.
    manager = pywrapcp.RoutingIndexManager(
        len(data['cost_matrix']), data['num_vehicles'],
        data['starts'], data['ends'])

    # Create Routing Model.
    routing = pywrapcp.RoutingModel(manager)

    # Create and register a transit callback.
    def distance_callback(from_index, to_index):
        """Returns the distance between the two nodes."""
        # Convert from routing variable Index to distance matrix NodeIndex.
        from_node = manager.IndexToNode(from_index)
        to_node = manager.IndexToNode(to_index)
        return data['cost_matrix'][from_node][to_node]

    transit_callback_index = routing.RegisterTransitCallback(distance_callback)

    # Define cost of each arc.
    routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index)

    # Add costs/distance constraint.
    matrix_costs = int(np.sum(data['cost_matrix']))
    dimension_name = 'Costs'
    routing.AddDimension(
        transit_callback_index,
        0,  # no slack
        matrix_costs,  # TODO reasonable max costs/distance
        True,  # start cumul to zero
        dimension_name)
    distance_dimension = routing.GetDimensionOrDie(dimension_name)
    distance_dimension.SetGlobalSpanCostCoefficient(100)

    # Define Transportation Requests.
    for request in data['pickups_deliveries']:
        pickup_index = manager.NodeToIndex(request[0])
        delivery_index = manager.NodeToIndex(request[1])
        routing.AddPickupAndDelivery(pickup_index, delivery_index)
        routing.solver().Add(
            routing.VehicleVar(pickup_index) == routing.VehicleVar(delivery_index))
        routing.solver().Add(
            distance_dimension.CumulVar(pickup_index) <=
            distance_dimension.CumulVar(delivery_index))

    # Add Capacity constraint.
    def demand_callback(from_index):
        """Returns the demand of the node."""
        # Convert from routing variable Index to demands NodeIndex.
        from_node = manager.IndexToNode(from_index)
        return data['demands'][from_node]

    demand_callback_index = routing.RegisterUnaryTransitCallback(demand_callback)
    routing.AddDimensionWithVehicleCapacity(
        demand_callback_index,
        0,  # null capacity slack
        data['vehicle_capacities'],  # vehicle maximum capacities
        True,  # start cumul to zero
        'Capacity')

    # Setting first solution heuristic.
    search_parameters = pywrapcp.DefaultRoutingSearchParameters()
    search_parameters.first_solution_strategy = (routing_enums_pb2.FirstSolutionStrategy.PARALLEL_CHEAPEST_INSERTION)
    search_parameters.time_limit.FromSeconds(time_limit_seconds)

    # Solve the problem.
    solution = routing.SolveWithParameters(search_parameters)

    if solution:
        return get_solution(data, manager, routing, solution, verbose)
    return None

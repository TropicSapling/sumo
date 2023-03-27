/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2023 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    MSPModel_JuPedSim.h
/// @author  Gregor Laemmel
/// @date    Mon, 13 Jan 2014
/// @author  Benjamin Cou�raud
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 27 Mar 2023
///
// The pedestrian following model that can instantiate different pedestrian models
// that come with the JuPedSim third-party simulation framework.
/****************************************************************************/
#pragma once
#include <config.h>

#include <memory>
#include <unordered_set>
#include <geos/geom/GeometryFactory.h>
#include <jupedsim/jupedsim.h>
#include "microsim/MSNet.h"
#include "MSPModel.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSPModel_JuPedSim
 * @brief A pedestrian following model that acts as a proxy for pedestrian models
 * provided by the JuPedSim third-party simulation framework.
 */
class MSPModel_JuPedSim : public MSPModel {
public:
    MSPModel_JuPedSim(const OptionsCont& oc, MSNet* net);
    ~MSPModel_JuPedSim();

    MSTransportableStateAdapter* add(MSTransportable* person, MSStageMoving* stage, SUMOTime now) override;
    void remove(MSTransportableStateAdapter* state) override;
    SUMOTime execute(SUMOTime time);

    bool usingInternalLanes();
    void registerArrived();
    int getActiveNumber();
    void clearState();

    enum PedestrianRoutingMode
    {
        JUPEDSIM_ROUTING,
        SUMO_ROUTING
    };

    class Event : public Command {
    public:
        explicit Event(MSPModel_JuPedSim* remoteModel)
            : myRemoteModel(remoteModel) { }
        SUMOTime execute(SUMOTime currentTime) override {
            return myRemoteModel->execute(currentTime);
        }

    private:
        MSPModel_JuPedSim* myRemoteModel;
    };

private:
    /**
    * @class PState
    * @brief Holds pedestrian state and performs updates
    */
    class PState : public MSTransportableStateAdapter {
    public:
        PState(MSPerson* person, MSStageMoving* stage, JPS_Journey journey, Position destination, JPS_AgentId agentId);
        ~PState() override;

        Position getPosition(const MSStageMoving& stage, SUMOTime now) const;
        void setPosition(double x, double y);

        double getAngle(const MSStageMoving& stage, SUMOTime now) const;
        void setAngle(double angle);

        MSStageMoving* getStage();
        MSPerson* getPerson();

        double getEdgePos(const MSStageMoving& stage, SUMOTime now) const;
        int getDirection(const MSStageMoving& stage, SUMOTime now) const;
        SUMOTime getWaitingTime(const MSStageMoving& stage, SUMOTime now) const;
        double getSpeed(const MSStageMoving& stage) const;
        const MSEdge* getNextEdge(const MSStageMoving& stage) const;
        Position getDestination(void) const;
        JPS_AgentId getAgentId(void) const;

    private:
        MSStageMoving* myStage;
        MSPerson* myPerson;
        Position myPosition;
        Position myDestination;
        double myAngle;
        JPS_Journey myJourney;
        JPS_AgentId myAgentId;
    };

    MSNet* myNetwork;
    int myNumActivePedestrians = 0;
    std::vector<PState*> myPedestrianStates;

    geos::geom::GeometryFactory::Ptr myGEOSGeometryFactory;
    std::vector<geos::geom::Geometry*> myGEOSLineStringsDump;
    std::vector<geos::geom::Geometry*> myGEOSPointsDump;
    std::vector<geos::geom::Geometry*> myGEOSBufferedGeometriesDump;
    std::vector<geos::geom::Geometry*> myGEOSGeometryCollectionsDump;
    std::vector<geos::geom::Geometry*> myGEOSConvexHullsDump;
    geos::geom::Geometry* myGEOSPedestrianNetwork;
    bool myIsPedestrianNetworkConnected;

    JPS_GeometryBuilder myJPSGeometryBuilder;
    JPS_Geometry myJPSGeometry;
    JPS_AreasBuilder myJPSAreasBuilder;
    JPS_Areas myJPSAreas;
    JPS_OperationalModel myJPSModel;
    JPS_ModelParameterProfileId myJPSParameterProfileId;
    JPS_Simulation myJPSSimulation;

    const PedestrianRoutingMode myRoutingMode = PedestrianRoutingMode::SUMO_ROUTING;

    static const int GEOS_QUADRANT_SEGMENTS;
    static const double GEOS_MIN_AREA;
    static const SUMOTime JPS_DELTA_T;
    static const double JPS_EXIT_TOLERANCE;

    void initialize();
    static MSLane* getNextPedestrianLane(const MSLane* const currentLane);
    
    static MSLane* getPedestrianLane(MSEdge* edge);
    static Position getAnchor(MSLane* lane, MSEdge* edge, ConstMSEdgeVector incoming);
    static Position getAnchor(MSLane* lane, MSEdge* edge, MSEdgeVector incoming);
    static std::tuple<ConstMSEdgeVector, ConstMSEdgeVector, std::unordered_set<MSEdge*>> getAdjacentEdgesOfJunction(MSJunction* junction);
    static const MSEdgeVector getAdjacentEdgesOfEdge(MSEdge* edge);
    static bool hasWalkingAreasInbetween(MSEdge* edge, MSEdge* otherEdge, ConstMSEdgeVector adjacentEdgesOfJunction);
    geos::geom::Geometry* createShapeFromCenterLine(PositionVector centerLine, double width, int capStyle);
    geos::geom::Geometry* createShapeFromAnchors(Position anchor, MSLane* lane, Position otherAnchor, MSLane* otherLane);
    geos::geom::Geometry* buildPedestrianNetwork(MSNet* network);
    static std::vector<double> getFlattenedCoordinates(const geos::geom::Geometry* geometry);
    void preparePolygonForJPS(const geos::geom::Polygon* polygon) const;
};

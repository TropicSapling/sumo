#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2021 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    test.py
# @author  Pablo Alvarez Lopez
# @date    2016-11-25

# import common functions for netedit tests
import os
import sys

testRoot = os.path.join(os.environ.get('SUMO_HOME', '.'), 'tests')
neteditTestRoot = os.path.join(
    os.environ.get('TEXTTEST_HOME', testRoot), 'netedit')
sys.path.append(neteditTestRoot)
import neteditTestFunctions as netedit  # noqa

# Open netedit
neteditProcess, referencePosition = netedit.setupAndStart(neteditTestRoot, ['--gui-testing-debug-gl'])

# go to additional mode
netedit.additionalMode()

# select trainStop
netedit.changeElement("trainStop")

# change reference to center
netedit.changeDefaultValue(9, "reference center")

# create trainStop in mode "reference center"
netedit.leftClick(referencePosition, 231, 238)

# change to move mode
netedit.moveMode()

# move trainStop to right
netedit.moveElement(referencePosition, 231, 260, 371, 260)

# go to inspect mode
netedit.inspectMode()

# inspect trainStop
netedit.leftClick(referencePosition, 371, 260)

# unblock additional
netedit.modifyBoolAttribute(13, False)

# change to move mode
netedit.moveMode()

# move trainStop to right
netedit.moveElement(referencePosition, 371, 260, 231, 260)

# Check undos and redos
netedit.undo(referencePosition, 3)
netedit.redo(referencePosition, 3)

# apply zoom
netedit.setZoom("25", "0", "74") 

# save additionals
netedit.saveAdditionals(referencePosition)

# save network
netedit.saveNetwork(referencePosition)

# quit netedit
netedit.quit(neteditProcess)

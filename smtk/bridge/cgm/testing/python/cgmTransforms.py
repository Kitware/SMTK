#!/usr/bin/python
import sys
#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
import smtk
from smtk.simple import *

mgr = smtk.model.Manager.create()
sref = mgr.createSession('cgm', smtk.model.SessionRef())
sref.assignDefaultName()
SetActiveSession(sref)

opnames = sref.operatorNames()

brick1 = CreateBrick(width=0.5)
brick2 = CreateBrick(width=0.5)

#json = smtk.io.ExportJSON.fromModelManager(mgr)
#jsonFile = open('/tmp/skirb1.json', 'w')
#print >> jsonFile, json
#jsonFile.close()

tr = sref.op('translate')
brick3 = Translate(brick2, [0.5, 0.0, 0.0])[0]
if not brick3 or brick3.entity() != brick2.entity():
  print "Expecting entities to match: %s != %s" % (brick2.entity(), brick3.entity())
  sys.exit(1)

brick4 = Rotate(brick3, angle=60.0, center=[0.5, 0.0, 0.0], axis=[0.3333, 0.6667, 0.6667])[0]
if not brick4 or brick4.entity() != brick3.entity():
  print "Expecting entities to match: %s != %s" % (brick3.entity(), brick4.entity())
  sys.exit(1)

brick5 = Union([brick1, brick4])
brick6 = Scale([brick5], [3.0, 2.0, 1.0])[0]

#json = smtk.io.ExportJSON.fromModelManager(mgr)
#jsonFile = open('/tmp/skirb4.json', 'w')
#print >> jsonFile, json
#jsonFile.close()

#
# Now verify that mgr.closeSession removes the entity record for the session.
mgr.closeSession(sref)
sys.exit(0 if sref.name() == ('invalid id ' + str(sref.entity())) else 1)

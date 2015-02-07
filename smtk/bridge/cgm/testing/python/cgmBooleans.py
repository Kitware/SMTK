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
sess = mgr.createSession('cgm')
brg = sess.session()
sess.assignDefaultName()
SetActiveSession(sess)
print '\n\n%s: type "%s" %s %s' % \
  (sess.name(), brg.name(), sess.flagSummary(0), brg.sessionId())
print '  Site: %s' % (sess.site() or 'local')

# We could evaluate the session tag as JSON, but most of
# the information is available through methods above that
# we needed to test:
sessiontag = sess.tag()
print '\n'

opnames = sess.operatorNames()

b0 = CreateBrick(center=[0,0,0])
Translate(b0, [0, 1.5, 0])
s0 = CreateSphere(center=[0.5,2.0,0.5], radius=0.5)
bsuni = Union(bodies=[b0, s0])

b0 = CreateBrick(center=[0,0,0])
s0 = CreateSphere(center=[0.5,0.5,0.5], radius=0.5)
bsint = Intersect(bodies=[b0, s0])

b0 = CreateBrick(center=[0,0,0])
s0 = CreateSphere(center=[0.5,0.5,0.5], radius=0.5)
bssub = Subtract(workpiece=b0, tool=s0)

Translate(bsint, [0.1, 0.1, 0.1])

json = smtk.io.ExportJSON.fromModelManager(mgr)
sphFile = open('boolean.json', 'w')
print >> sphFile, json
sphFile.close()

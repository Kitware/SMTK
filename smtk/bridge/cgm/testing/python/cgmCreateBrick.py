#!/usr/bin/python
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
mgr = smtk.model.Manager.create()
sess = mgr.createSession('cgm')
brg = sess.bridge()

cb = sess.op('create brick')
ov = cb.findInt('operand variants')
ctr = cb.findDouble('center')
#cb.findDouble('radius').setValue(0.5)
res = cb.operate()
brick = res.findModelEntity('bodies').value(0)

json = smtk.io.ExportJSON.fromModel(mgr)
sphFile = open('/tmp/brickly.json', 'w')
print >> sphFile, json
sphFile.close()

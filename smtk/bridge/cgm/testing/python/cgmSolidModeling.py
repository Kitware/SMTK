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
try:
  import cgmsmtk
except:
  print "Expected"

mgr = smtk.model.Manager.create()
brg = smtk.model.Manager.createBridge('cgm')
brg.name()
brg.sessionId()
mgr.registerBridgeSession(brg)

opnames = brg.operatorNames()
cs1 = brg.op('create sphere', mgr)
cs1.findDouble('radius').setValue(1.)
#cs1.findDouble('inner radius').setValue(0.1) # Crashes
#cs1.findDouble('inner radius').setValue(-0.1) # Complains bitterly
cs1.findDouble('inner radius').setValue(0.2) # Actually works

# CGM's OCC backend apparently does not pay attention to
# the sphere center parameters:
cs1.findDouble('center').setValue(0, 0.2)
cs1.findDouble('center').setValue(1, 0.2)
cs1.findDouble('center').setValue(2, 0.2)

res = cs1.operate()
sph = res.findModelEntity('bodies').value(0)

cs2 = brg.op('create sphere', mgr)
cs2.findDouble('radius').setValue(0.5)
res2 = cs2.operate()
sph2 = res2.findModelEntity('bodies').value(0)

u1 = brg.op('union', mgr)
u1.associateEntity(sph)
u1.associateEntity(sph2)
res = u1.operate()
# You will see:
#    Updated volume(s): 2
#    Destroyed volume(s): 1
su = res.findModelEntity('bodies').value(0)
# Note that su has same UUID as sph2

#json = smtk.io.ExportJSON.fromModel(mgr)
#sphFile = open('/tmp/s3.json', 'w')
#print >> sphFile, json
#sphFile.close()

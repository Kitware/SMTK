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
try:
  import cgmsmtk
except:
  print 'Expecting ERROR:'

mgr = smtk.model.Manager.create()
sess = mgr.createSession('cgm')
brg = sess.bridge() # smtk.model.Manager.createBridge('cgm')
#sess = smtk.model.BridgeSession(mgr, brg)
sess.assignDefaultName()
print '\n\n%s: type "%s" %s %s' % \
  (sess.name(), brg.name(), sess.flagSummary(0), brg.sessionId())
print '  Site: %s' % (sess.site() or 'local')
for eng in sess.engines():
  print '  Engine %s filetypes:\n    %s' % \
    (eng, '\n    '.join(sess.fileTypes(eng)))
# We could evaluate the session tag as JSON, but most of
# the information is available through methods above that
# we needed to test:
bridgetag = sess.tag()
print '\n'

opnames = sess.operatorNames()
cs1 = sess.op('create sphere')
cs1.findAsDouble('radius').setValue(1.)
#cs1.findAsDouble('inner radius').setValue(0.1) # Crashes
#cs1.findAsDouble('inner radius').setValue(-0.1) # Complains bitterly
cs1.findAsDouble('inner radius').setValue(0.2) # Actually works

# CGM's OCC backend apparently does not pay attention to
# the sphere center parameters:
cs1.findAsDouble('center').setValue(0, 0.2)
cs1.findAsDouble('center').setValue(1, 0.2)
cs1.findAsDouble('center').setValue(2, 0.2)

res = cs1.operate()
sph = res.findModelEntity('bodies').value(0)

cs2 = sess.op('create sphere')
cs2.findAsDouble('radius').setValue(0.5)
res2 = cs2.operate()
sph2 = res2.findModelEntity('bodies').value(0)

print 'Operators that can associate with ' + sph2.flagSummary(1) + ' include\n  %s' % \
  '\n  '.join(sess.operatorsForAssociation(sph2.entityFlags()))

u1 = sess.op('union')
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

#
# Now verify that mgr.closeSession removes the entity record for the session.
mgr.closeSession(sess)
sys.exit(0 if sess.name() == ('invalid id ' + str(sess.entity())) else 1)

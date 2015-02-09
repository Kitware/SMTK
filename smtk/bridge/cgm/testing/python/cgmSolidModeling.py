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

mgr = smtk.model.Manager.create()
sref = mgr.createSession('cgm', smtk.model.SessionRef())
sess = sref.session()
sref.assignDefaultName()
print '\n\n%s: type "%s" %s %s' % \
  (sref.name(), sess.name(), sref.flagSummary(0), sess.sessionId())
print '  Site: %s' % (sref.site() or 'local')
for eng in sref.engines():
  print '  Engine %s filetypes:\n    %s' % \
    (eng, '\n    '.join(sref.fileTypes(eng)))
# We could evaluate the session tag as JSON, but most of
# the information is available through methods above that
# we needed to test:
sessiontag = sref.tag()
print '\n'

opnames = sref.operatorNames()
cs1 = sref.op('create sphere')
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
sph = res.findModelEntity('entities').value(0)

cs2 = sref.op('create sphere')
cs2.findAsDouble('radius').setValue(0.5)
cs2.findAsDouble('center').setValue(0, 0.9)
res2 = cs2.operate()
sph2 = res2.findModelEntity('entities').value(0)

print 'Operators that can associate with ' + sph2.flagSummary(1) + ' include\n  %s' % \
  '\n  '.join(sref.operatorsForAssociation(sph2.entityFlags()))

u1 = sref.op('union')
u1.associateEntity(sph)
u1.associateEntity(sph2)
res = u1.operate()
# You will see:
#    Updated volume(s): 2
#    Destroyed volume(s): 1
su = res.findModelEntity('entities').value(0)
# Note that su has same UUID as sph2


# Test cylinder creation.
from smtk.simple import *
SetActiveSession(sref)
cyl = CreateCylinder(top_radius=1.0)

#json = smtk.io.ExportJSON.fromModelManager(mgr)
#cylFile = open('cyl.json', 'w')
#print >> cylFile, json
#cylFile.close()


#
# Now verify that mgr.closeSession removes the entity record for the session.
mgr.closeSession(sref)
sys.exit(0 if sref.name() == ('invalid id ' + str(sref.entity())) else 1)

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

def sumCond(itm, idx):
  """Set a conditional item."""
  print itm.name(), '=', idx
  itm.setDiscreteIndex(idx)
  for i in range(itm.numberOfActiveChildrenItems()):
    citm = smtk.attribute.to_concrete(itm.activeChildItem(i)) #
    print '  ', citm.name(), '=', ' '.join([str(citm.value(j)) for j in range(citm.numberOfValues())])

def setVector(ax,v):
  """Set a vector-valued attribute given a Python list or tuple."""
  for i in range(len(v)):
    ax.setValue(i,v[i])


mgr = smtk.model.Manager.create()
sref = mgr.createSession('cgm', smtk.model.SessionRef())
sref.assignDefaultName()

opnames = sref.operatorNames()
cb = sref.op('create brick')
ov = cb.findAsInt('construction method')
ov.setDiscreteIndex(0)
cb.findAsDouble('width').setValue(0.5)
r1 = cb.operate()
brick1 = r1.findModelEntity('entities').value(0)

r2 = cb.operate()
brick2 = r2.findModelEntity('entities').value(0)

#json = smtk.io.ExportJSON.fromModelManager(mgr)
#jsonFile = open('/tmp/skirb1.json', 'w')
#print >> jsonFile, json
#jsonFile.close()

tr = sref.op('translate')
tr.associateEntity(brick2)
off = tr.findAsDouble('offset')
setVector(off, [.5, 0., 0.])
r3 = tr.operate()
brick3 = r3.findModelEntity('entities').value(0)


if not brick3 or brick3.entity() != brick2.entity():
  print "Expecting entities to match: %s != %s" % (brick2.entity(), brick3.entity())
  sys.exit(1)

ro = sref.op('rotate')
ro.associateEntity(brick3)
ctr = ro.findAsDouble('center')
axs = ro.findAsDouble('axis')
ang = ro.findAsDouble('angle')
setVector(ctr, [.5, 0., 0.])
setVector(axs, [.3333, .6667, 0.6667])
ang.setValue(0, 60.0)
r4 = ro.operate()
brick4 = r4.findModelEntity('entities').value(0)


if not brick4 or brick4.entity() != brick3.entity():
  print "Expecting entities to match: %s != %s" % (brick3.entity(), brick4.entity())
  sys.exit(1)

un = sref.op('union')
un.associateEntity(brick1)
un.associateEntity(brick4)
r5 = un.operate()
brick5 = r5.findModelEntity('entities').value(0)

sc = sref.op('scale')
sc.associateEntity(brick5)
sc.findAsInt('scale factor type').setDiscreteIndex(0)
sc.findAsDouble('scale factor').setValue(3.0)
r6 = sc.operate()
brick6 = r6.findModelEntity('entities').value(0)

#json = smtk.io.ExportJSON.fromModelManager(mgr)
#jsonFile = open('/tmp/skirb4.json', 'w')
#print >> jsonFile, json
#jsonFile.close()

#
# Now verify that mgr.closeSession removes the entity record for the session.
mgr.closeSession(sref)
sys.exit(0 if sref.name() == ('invalid id ' + str(sref.entity())) else 1)

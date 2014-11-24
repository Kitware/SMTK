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
ov = cb.findAsInt('construction method')
ov.setDiscreteIndex(0)
ctr = cb.findAsDouble('center')
cb.findAsDouble('width').setValue(0.5)
res = cb.operate()
brick = res.findModelEntity('bodies').value(0)

def sumCond(itm, idx):
  print itm.name(), '=', idx
  itm.setDiscreteIndex(idx)
  for i in range(itm.numberOfActiveChildrenItems()):
    citm = smtk.attribute.to_concrete(itm.activeChildItem(i)) #
    print '  ', citm.name(), '=', ' '.join([str(citm.value(j)) for j in range(citm.numberOfValues())])

def setAxis(ax,v):
  for i in range(len(v)):
    ax.setValue(i,v[i])

ov.setDiscreteIndex(1)
ov.numberOfActiveChildrenItems()
ax0 = cb.findAsDouble('axis 0')
ax1 = cb.findAsDouble('axis 1')
ax2 = cb.findAsDouble('axis 2')
setAxis(ax0,[1,1,0])
setAxis(ax1,[-1,1,0])
setAxis(ax2,[0,0,1])
ext = cb.findAsDouble('extension')
setAxis(ext,[2, 3, .2])
sumCond(ov,1)
r2 = cb.operate()
b2 = r2.findModelEntity('bodies').value(0)

uop = sess.op('union')
uop.associateEntity(brick)
uop.associateEntity(b2)
r3 = uop.operate()
ubod = r3.findModelEntity('bodies').value(0)

top = sess.op('translate')
top.associateEntity(ubod)
off = top.findAsDouble('offset')
setAxis(off, [8., 3., 7.])
r4 = top.operate()
b4 = r4.findModelEntity('bodies').value(0)

json = smtk.io.ExportJSON.fromModel(mgr)
sphFile = open('/tmp/brickly2.json', 'w')
print >> sphFile, json
sphFile.close()

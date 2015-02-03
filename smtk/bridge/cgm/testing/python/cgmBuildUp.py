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
sref = mgr.createSession('cgm', smtk.model.SessionRef())

def setCoord(x,v):
  for i in range(len(v)):
    x.setValue(i,v[i])

def setEntitiesByIndex(p,ep,v):
  for i in range(len(ep)):
    p.setValue(i, v[ep[i]])

verts = []
edges = []
faces = []
volus = []

pcoords = [
    (0,0,0),
    (1,0,0),
    (0,1,0),
    (0,0,1)]
crv = sref.op('create vertex')
x = crv.findAsDouble('point')
c = crv.findAsInt('color')
c.setValue(0, 1)
for pt in pcoords:
  setCoord(x,pt);
  verts.append(crv.operate().findModelEntity('vertex').value(0))

epts = [
    (0,1),
    (0,2),
    (0,3),
    (1,2),
    (1,3),
    (2,3)]

cre = sref.op('create edge')
t = cre.findAsInt('curve type')
t.setValue(0,6) # 6 == line segment
v = cre.findAsModel('vertices')
x = cre.findAsDouble('point')
c = cre.findAsInt('color')
c.setValue(0, 2)
for epair in epts:
  setEntitiesByIndex(v,epair,verts)
  if epair == (2,3):
    # Make the last edge an arc:
    t.setValue(0,1) # 1 == arc
    setCoord(x,[0,0.6,0.6]) # third point on arc
  edges.append(cre.operate().findModelEntity('edge').value(0))


fedg = [
    (12, 0, 3, 1),
    (12, 0, 4, 2),
    (16, 1, 5, 2)
    ]
#   (16, 3, 5, 4) # <-- OpenCascade cannot infer that this face should be cylindrical
crf = sref.op('create face')
t = crf.findAsInt('surface type')
t.setValue(0, 12)
e = crf.findAsModel('edges')
c = crf.findAsInt('color')
c.setValue(0, 3)
for face in fedg:
  e.setNumberOfValues(len(face)-1)
  setEntitiesByIndex(e,face[1:],edges)
  t.setValue(face[0])
  faces.append(crf.operate().findModelEntity('face').value(0))

json = smtk.io.ExportJSON.fromModelManager(mgr)
sphFile = open('buildup.json', 'w')
print >> sphFile, json
sphFile.close()

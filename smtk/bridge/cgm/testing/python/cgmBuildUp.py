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
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.bridge.cgm
    import smtk.io
from smtk.simple import *

# def SetVectorValue(x,v):
#  x.setNumberOfValues(len(v))
#  for i in range(len(v)):
#    x.setValue(i,v[i])


def MapOffsets(vals, idxs):
    return [vals[i] for i in idxs]


pcoords = [
    (0, 0, 0),
    (1, 0, 0),
    (0, 1, 0),
    (0, 0, 1)]

epts = [
    (0, 1),
    (0, 2),
    (0, 3),
    (1, 2),
    (1, 3),
    (2, 3)]

fedg = [
    (SurfaceType.PLANAR, 0, 3, 1),
    (SurfaceType.PLANAR, 0, 4, 2),
    (SurfaceType.PLANAR, 1, 5, 2)
]
#   (SurfaceType.CYLINDRICAL, 3, 5, 4) # <-- OpenCascade cannot infer that this face should be cylindrical

mgr = smtk.model.Manager.create()
sref = mgr.createSession('cgm', smtk.model.SessionRef())
SetActiveSession(sref)

verts = []
edges = []
faces = []
bodies = []

for pt in pcoords:
    verts.append(CreateVertex(pt, color=1))

for epair in epts:
    if epair == (2, 3):
        edges.append(CreateEdge([verts[i] for i in epair],
                                CurveType.ARC, midpoint=[0, 0.6, 0.6]))
    else:
        edges.append(CreateEdge([verts[i] for i in epair]))

for face in fedg:
    faces.append(CreateFace([edges[i]
                             for i in face[1:]], surface_type=face[0]))

for face in faces:
    bodies.append(Sweep([face, ], SweepType.EXTRUDE, distance=0.1))

Write('buildup.brep', bodies)

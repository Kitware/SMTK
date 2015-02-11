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
"""
:mod:`smtk.simple` --- A Simple Modeling Interface
==================================================

.. module:: smtk.simple
   :synopsis: A basic geometric modeling kernel API.
.. moduleauthor:: Kitware, Inc. <kitware@kitware.com>

This module provides a set of simple (non-class based)
functions for interacting with solid modeling kernels,
in the spirit of ParaView's simple module.

You are expected to set the active session to one that
uses the modeling kernel you want to target. Use the
SetActiveSession() method for that::

  import smtk
  from smtk.simple import *

  mgr = smtk.model.Manager.create()
  sess = mgr.createSession('cgm')
  SetActiveSession(sess)

Once the session is set, subsequent modeling operations
will use its operators. ::

  b1 = CreateBrick()
  Translate(b1, [5,0,0])
  b2 = CreateBrick(depth=2)
"""
import smtk

activeSession = None

def SetActiveSession(sess):
  """Set the session to be used when performing modeling operations."""
  global activeSession
  activeSession = sess

def GetActiveSession():
  """Return the currently-active modeling session."""
  global activeSession
  return activeSession

class CurveType:
  ARC = 1
  LINE = 6

class SurfaceType:
  PLANAR = 12
  CYLINDRICAL = 16

class SweepType:
  EXTRUDE = 0
  REVOLVE = 1
  HELIX = 2
  ALONG_PATH = 3

class DraftType:
  RECTANGULAR = 1
  ROUNDED = 2

def SetVectorValue(item,v):
  """Given an smtk.attribute.Item, set its values to the entries of the list v.

  This will fail if the number of values in the list v
  is not acceptable according to item's definition, or
  if the values in v cannot be converted to the proper
  type.
  """
  item.setNumberOfValues(len(v))
  for i in range(len(v)):
    item.setValue(i,v[i])

def GetVectorValue(item):
  """Given an smtk.attribute.Item, return a list containing its values."""
  N = item.numberOfValues()
  return [item.value(i) for i in range(N)]

def PrintResultLog(res, always = False):
  """Given an operator result, print log messages if unsuccessful."""
  if always or res.findInt('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
    slog = res.findString('log')
    print '\n'.join([slog.value(i) for i in range(slog.numberOfValues())])

def CreateSphere(**args):
  """Create a sphere.

  This method accepts optional radius, center, and inner_radius
  arguments and returns a ModelEntity cursor for the created
  sphere.

  Example
  -------

  .. code:: python

      sphere = CreateSphere(radius=1.0, center=[1., 1., 1.])

  When unspecified, arguments take on their default values.
  """
  sess = GetActiveSession()
  cs = sess.op('create sphere')
  if 'radius' in args:
    cs.findAsDouble('radius').setValue(args['radius'])
  if 'inner_radius' in args:
    cs.findAsDouble('inner radius').setValue(args['inner_radius'])
  if 'center' in args:
    cc = cs.findAsDouble('center')
    SetVectorValue(cc, args['center'])
  res = cs.operate()
  PrintResultLog(res)
  sph = res.findModelEntity('entities').value(0)
  return sph

def CreateCylinder(**args):
  """Create a generalized cylinder (a truncated elliptical cone).

  This method accepts optional `height`, `radius`, `minor_radius`,
  and `top_radius` arguments; it returns an EntityRef pointing to the
  new cylinder.

  The cylinder base is always at the origin and has its axis of
  radial symmetry on the z axis. Use the Translate() and Rotate()
  operators to place the cylinder as required.

  Example
  -------

  .. code:: python

      cylinder = CreateCylinder(radius=1.0, height=1.0)

  When unspecified, arguments take on their default values:

  * `height` defaults to 1.
  * `radius` defaults to 1.
  * `minor_radius` defaults to 1.
  * `top_radius` defaults to 0.
  """
  sess = GetActiveSession()
  cs = sess.op('create cylinder')
  if 'radius' in args:
    cs.findAsDouble('major base radius').setValue(args['radius'])
  if 'minor_radius' in args:
    cs.findAsDouble('minor base radius').setValue(args['minor_radius'])
  if 'top_radius' in args:
    cs.findAsDouble('major top radius').setValue(args['top_radius'])
  if 'height' in args:
    cs.findAsDouble('height').setValue(args['height'])
  res = cs.operate()
  PrintResultLog(res)
  cyl = res.findModelEntity('entities').value(0)
  return cyl

def CreateBrick(**args):
  """Create a rectangular cuboid (6 rectangular faces).

  The brick center may always be specified and defaults
  to the origin if unspecified.
  Bricks may be specified either by width, height, and
  depth; or by passing 3 axes and a vector of extents
  along each axis. The axes are orthogonalized before
  construction using the Gram-Schmidt process.
  This method returns a ModelEntity cursor for the created
  brick.

  Example
  -------

  .. code:: python

      brick = CreateBrick(width=2, height=3, depth=1)
      brick = CreateBrick( \
        axis_0=[1,0,0], axis_1=[0,1,0], \
        axis_2=[0,0,1], ext=[2,3,1])

  When unspecified, arguments take on their default values.
  """
  sess = GetActiveSession()
  cb = sess.op('create brick')
  meth = 1 if 'ext' in args else 0
  ov = cb.findAsInt('construction method')
  ov.setDiscreteIndex(meth)
  if meth == 0:
    for prop in ['width', 'depth', 'height']:
      if prop in args:
        cb.findAsDouble(prop).setValue(args[prop])
  else:
    extVal = args['ext']
    SetVectorValue(cb.findAsDouble('extension'), extVal)
    for prop in ['axis 0', 'axis 1', 'axis 2']:
      if prop in args:
        propVal = args[prop]
        SetVectorValue(cb.findAsDouble(prop), propVal)
  # Either construction method can specify 'center':
  if 'center' in args:
    ctrVal = args['center']
    SetVectorValue(cb.findAsDouble('center'), ctrVal)
  res = cb.operate()
  PrintResultLog(res)
  brick = res.findModelEntity('entities').value(0)
  return brick

def Intersect(bodies, **args):
  """Compute the boolean intersection of a set of bodies.

  Example
  -------

  .. code:: python

      sphere = CreateSphere(radius=1.0, center=[1., 1., 1.])
      brick = CreateBrick(width=2, height=3, depth=1)
      intersected = Intersect([sphere, brick])
  """
  sess = GetActiveSession()
  op = sess.op('intersection')
  try:
    [op.associateEntity(x) for x in bodies]
  except:
    op.associateEntity(bodies)
  res = op.operate()
  PrintResultLog(res)
  return res.findModelEntity('entities').value(0)

def Union(bodies, **args):
  """Compute the boolean union of a set of bodies.

  Example
  -------

  .. code:: python

      sphere = CreateSphere(radius=1.0, center=[1., 1., 1.])
      brick = CreateBrick(width=2, height=3, depth=1)
      blob = Union([sphere, brick])
  """
  sess = GetActiveSession()
  op = sess.op('union')
  try:
    [op.associateEntity(x) for x in bodies]
  except:
    op.associateEntity(bodies)
  res = op.operate()
  PrintResultLog(res)
  return res.findModelEntity('entities').value(0)

def Subtract(workpiece, tool, **args):
  """Perform a boolean subtraction of the tool from the workpiece.

  Tool and workpiece may each be either an SMTK model or a list of models.

  Example
  -------

  .. code:: python

      sphere = CreateSphere(radius=1.0, center=[1., 1., 1.])
      brick = CreateBrick(width=2, height=3, depth=1)

      # Subtract the tool from the workpiece (we are left
      # with the portion of the brick outside the sphere):
      diff = Subtract(tool=sphere, workpiece=brick)
  """
  sess = GetActiveSession()
  op = sess.op('subtraction')
  if type(workpiece) == type([]):
    [op.associateEntity(x) for x in workpiece]
  else:
    op.associateEntity(workpiece)

  # Convert tool to a list
  if type(tool) != type([]):
    tool = [tool,]
  SetVectorValue(op.findModelEntity('tools',smtk.attribute.ALL_CHILDREN), tool)

  res = op.operate()
  PrintResultLog(res)
  return res.findModelEntity('entities').value(0)

def Translate(bodies, vec):
  """Translate the body (or list of bodies) along the given vector."""
  sess = GetActiveSession()
  top = sess.op('translate')
  if type(bodies) == type([]):
    [top.associateEntity(bod) for bod in bodies]
  else:
    top.associateEntity(bodies)
  SetVectorValue(top.findAsDouble('offset'),vec)
  res = top.operate()
  PrintResultLog(res)
  return GetVectorValue(res.findModelEntity('entities'))

def CreateVertex(pt, **kwargs):
  """Create a vertex given point coordinates.
  """
  sref = GetActiveSession()
  crv = sref.op('create vertex')
  x = crv.findAsDouble('point')
  c = crv.findAsInt('color')
  if c and 'color' in kwargs:
    c.setValue(0, kwargs['color'])
  SetVectorValue(x, pt)
  res = crv.operate()
  PrintResultLog(res)
  return res.findModelEntity('entities').value(0)

def CreateEdge(verts, curve_type = CurveType.LINE, **kwargs):
  """Create an edge from a pair of vertices.
  """
  sref = GetActiveSession()
  cre = sref.op('create edge')
  [cre.associateEntity(x) for x in verts]
  t = cre.findAsInt('curve type')
  t.setValue(0, curve_type)
  if 'midpoint' in kwargs:
    x = cre.findAsDouble('point')
    SetVectorValue(x, kwargs['midpoint'])
  if 'color' in kwargs:
    c = cre.findAsInt('color')
    c.setValue(0, kwargs['color'])
  res = cre.operate()
  PrintResultLog(res)
  return res.findModelEntity('entities').value(0)

def CreateFace(edges, surface_type = SurfaceType.PLANAR, **kwargs):
  """Create a face from a set of edges.
  """
  sref = GetActiveSession()
  crf = sref.op('create face')
  [crf.associateEntity(x) for x in edges]
  t = crf.findAsInt('surface type')
  t.setValue(0, surface_type)
  if 'color' in kwargs:
    c = crf.findAsInt('color')
    c.setValue(0, kwargs['color'])
  res = crf.operate()
  PrintResultLog(res)
  return res.findModelEntity('entities').value(0)

def CreateBody(ents, **kwargs):
  """Create a set of bodies from a set of cells.

  This is used to create sheet bodies (one per face) when given faces
  or a single solid body when given volumes.
  If given edges, it will attempt to create a planar face and then a sheet body.
  Any combination of edges, faces, and volumes may be given;
  they are processed independently.
  """
  sref = GetActiveSession()
  crb = sref.op('create body')
  [crb.associateEntity(x) for x in ents]
  if 'color' in kwargs:
    c = crb.findAsInt('color')
    c.setValue(0, kwargs['color'])
  if 'keep_inputs' in kwargs:
    c = crb.findAsInt('keep inputs')
    c.setValue(0, kwargs['keep_inputs'])
  res = crb.operate()
  PrintResultLog(res)
  bodies = res.findModelEntity('entities')
  return [bodies.value(i) for i in range(bodies.numberOfValues())]

def Sweep(stuffToSweep, method = SweepType.EXTRUDE, **kwargs):
  """Sweep curves or surfaces to create a model.
  """
  sref = GetActiveSession()
  swp = sref.op('sweep')
  [swp.associateEntity(x) for x in stuffToSweep]
  meth = swp.findAsInt('construction method').setDiscreteIndex(method)
  if method == SweepType.EXTRUDE:
    if 'direction' in kwargs:
      axis = swp.findAsDouble('extrusion direction')
      SetVectorValue(axis, kwargs['direction'])
      axis.setIsEnabled(True)
    if 'distance' in kwargs:
      dist = swp.findAsDouble('sweep distance')
      dist.setValue(kwargs['distance'])
      dist.setIsEnabled(True)
    if 'draft_angle' in kwargs:
      draft_angle = swp.findAsDouble('draft angle').setValue(0, kwargs['draft_angle'])
    if 'draft_type' in kwargs:
      draft_type  = swp.findAsInt('draft type').setValue(0, kwargs['draft_type'])
  elif method == SweepType.REVOLVE:
    if 'axis' in kwargs:
      axis = swp.findAsDouble('axis of revolution')
      SetVectorValue(axis, kwargs['axis'])
    if 'base_point' in kwargs:
      base = swp.findAsDouble('axis base point')
      SetVectorValue(base, kwargs['base_point'])
    if 'sweep_angle' in kwargs:
      angl = swp.findAsDouble('sweep angle').setValue(0, kwargs['sweep_angle'])
  elif method == SweepType.HELIX:
    if 'axis' in kwargs:
      axis = swp.findAsDouble('axis of revolution')
      SetVectorValue(axis, kwargs['axis'])
    if 'base_point' in kwargs:
      base = swp.findAsDouble('axis base point')
      SetVectorValue(base, kwargs['base_point'])
    if 'helix_angle' in kwargs:
      angl = swp.findAsDouble('helix angle').setValue(0, kwargs['helix_angle'])
    if 'pitch' in kwargs:
      pitch = swp.findAsDouble('pitch').setValue(0, kwargs['pitch'])
    if 'handedness' in kwargs:
      angl = swp.findAsInt('handedness').setValue(0, kwargs['handedness'])
  res = swp.operate()
  PrintResultLog(res)
  return res.findModelEntity('entities').value(0)

def Write(filename, entities = [], **kwargs):
  """Write a set of entities to an file (in the native modeling kernel format).
  """
  sref = GetActiveSession()
  wri = sref.op('write')
  wri.findAsFile('filename').setValue(0, filename)
  [wri.associateEntity(entity) for entity in entities]
  if 'filetype' in kwargs:
    wri.findAsString('filetype').setValue(0, kwargs['filetype'])
  res = wri.operate()
  PrintResultLog(res)
  return res.findInt('outcome').value(0)

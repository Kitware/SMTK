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
import os
import smtk

activeSession = None
lastOperatorResult = None

def SetActiveSession(sess):
  """Set the session to be used when performing modeling operations."""
  global activeSession
  activeSession = sess

def GetActiveSession():
  """Return the currently-active modeling session."""
  global activeSession
  return activeSession

def SetLastResult(res):
  """An internal method used by operations to save their results for advanced users."""
  global lastOperatorResult
  lastOperatorResult = res

def GetLastResult():
  """Returns the result of the last operation invoked via the smtk.simple API."""
  global lastOperatorResult
  return lastOperatorResult

class CurveType:
  ARC = 1
  LINE = 6

class SurfaceType:
  PLANAR = 12
  CYLINDRICAL = 16

class ScaleType:
  UNIFORM = 0
  PER_AXIS = 1

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
  try:
    item.setNumberOfValues(len(v))
    for i in range(len(v)):
      item.setValue(i,v[i])
  except: # Maybe v is a scalar
    item.setNumberOfValues(1)
    item.setValue(0, v)

def GetVectorValue(item):
  """Given an smtk.attribute.Item, return a list containing its values."""
  N = item.numberOfValues()
  return [item.value(i) for i in range(N)]

def PrintResultLog(res, always = False):
  """Given an operator result, print log messages if unsuccessful."""
  if always or res.findInt('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
    slog = res.findString('log')
    tmplog = smtk.io.Logger()
    if slog.numberOfValues() < 1:
      return
    smtk.io.ImportJSON.ofLog(slog.value(0), tmplog)
    print '\n'.join(
        [
          tmplog.severityAsString(tmplog.record(i).severity) + ': ' +
          tmplog.record(i).message for i in range(tmplog.numberOfRecords())])
    #print '\n'.join([slog.value(i) for i in range(slog.numberOfValues())])

def CreateModel(**args):
  """Create an empty geometric model.
  """
  sess = GetActiveSession()
  cm = sess.op('create model')
  if cm is None:
    return
  xAxis = args['x_axis'] if 'x_axis' in args else None
  yAxis = args['y_axis'] if 'y_axis' in args else None
  normal = args['normal'] if 'normal' in args else (args['z_axis'] if 'z_axis' in args else None)
  origin = args['origin'] if 'origin' in args else None
  modelScale = args['model_scale'] if 'model_scale' in args else None
  featureSize = args['feature_size'] if 'feature_size' in args else None
  if modelScale is not None and featureSize is not None:
    print 'Specify either model_scale or feature_size but not both'
    return
  method = -1
  if modelScale is not None:
    if normal is not None:
      print 'When specifying model_scale, you must specify x and y axes. Normal is ignored.'
    method = 2
  if featureSize is not None:
    if normal is not None:
      method = 1
    else:
      method = 0
  cm.findAsInt('construction method').setDiscreteIndex(method)
  if origin is not None:
    SetVectorValue(cm.findAsDouble('origin'), origin)
  if xAxis is not None:
    SetVectorValue(cm.findAsDouble('x axis'), xAxis)
  if yAxis is not None:
    SetVectorValue(cm.findAsDouble('y axis'), yAxis)
  if normal is not None:
    SetVectorValue(cm.findAsDouble('z axis'), normal)
  if modelScale is not None:
    SetVectorValue(cm.findAsInt('model scale'), modelScale)
  if featureSize is not None:
    SetVectorValue(cm.findAsDouble('feature size'), featureSize)
  res = cm.operate()
  SetLastResult(res)
  PrintResultLog(res)
  mod = res.findModelEntity('created').value(0)
  return mod

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
  SetLastResult(res)
  PrintResultLog(res)
  sph = res.findModelEntity('created').value(0)
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
  SetLastResult(res)
  PrintResultLog(res)
  cyl = res.findModelEntity('created').value(0)
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
  SetLastResult(res)
  PrintResultLog(res)
  brick = res.findModelEntity('created').value(0)
  return brick

def Delete(ents, **kwargs):
  """Delete entities and (optionally) their bordant entities.

  To delete dependent entities, pass delete_dependents=True.
  """
  sess = GetActiveSession()
  op = sess.op('delete')
  SetVectorValue(op.specification().associations(), ents)
  if 'delete_dependents' in kwargs and kwargs['delete_dependents']:
    op.findVoid('delete dependents', int(smtk.attribute.ALL_CHILDREN)).setIsEnabled()
  res = op.operate()
  SetLastResult(res)
  PrintResultLog(res)
  exp = res.findModelEntity('expunged')
  return [exp.value(i) for i in range(exp.numberOfValues())]


def ImportSMTKModel(filename):
  """Import an SMTK model into the active session."""
  sess = GetActiveSession()
  op = sess.op('import smtk model')
  if smtk.wrappingProtocol() == 'pybind11':
    fname = op.findFile('filename', int(smtk.attribute.ALL_CHILDREN))
  else:
    fname = op.findFile('filename', smtk.attribute.ALL_CHILDREN)
  fname.setValue(filename)
  res = op.operate()
  SetLastResult(res)
  PrintResultLog(res)
  cre = res.findModelEntity('created')
  return [cre.value(i) for i in range(cre.numberOfValues())]

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
  SetLastResult(res)
  PrintResultLog(res)
  return res.findModelEntity('modified').value(0)

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
  SetLastResult(res)
  PrintResultLog(res)
  return res.findModelEntity('modified').value(0)

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
  if smtk.wrappingProtocol() == 'pybind11':
    SetVectorValue(op.findModelEntity('tools',int(smtk.attribute.ALL_CHILDREN)), tool)
  else:
    SetVectorValue(op.findModelEntity('tools',smtk.attribute.ALL_CHILDREN), tool)

  res = op.operate()
  SetLastResult(res)
  PrintResultLog(res)
  return res.findModelEntity('modified').value(0)

def Rotate(bodies, **args):
  """Rotate the body (or list of bodies) through an angle about an axis passing through the center point.

  The angle must be specified in degrees, not radians.
  There are no defaults for the axis and center; you must
  specify them.

  Example
  -------

  .. code:: python

      brick = CreateBrick(width=2, height=3, depth=1)
      result = Rotate(brick, angle=60., axis=[0.333, 0.667, 0.667], center=[0.5, 0, 0])
      # Note that result is a list, but you should expect 1 output for each input:
      rotated = result[0]
  """
  sess = GetActiveSession()
  rop = sess.op('rotate')
  if type(bodies) == type([]):
    [rop.associateEntity(bod) for bod in bodies]
  else:
    rop.associateEntity(bodies)
  SetVectorValue(rop.findAsDouble('center'), args['center'])
  SetVectorValue(rop.findAsDouble('axis'), args['axis'])
  if 'angle' in args:
    rop.findAsDouble('angle').setValue(args['angle'])
  res = rop.operate()
  SetLastResult(res)
  PrintResultLog(res)
  return GetVectorValue(res.findModelEntity('modified'))

def Scale(bodies, factor, **kwargs):
  """Scale a model.

  For uniform scaling along every direction, simply pass a
  'factor' argument specifying the scale factor as a single
  number.
  Otherwise, specify 'factor' as a list of 3 numbers.
  An origin may also be specified as a list of 3 numbers.
  """
  sref = GetActiveSession()
  sca = sref.op('scale')
  if isinstance(bodies,list):
    [sca.associateEntity(x) for x in bodies]
  else:
    sca.associateEntity(bodies)
  from numbers import Number
  method = ScaleType.UNIFORM if isinstance(factor, Number) else ScaleType.PER_AXIS
  meth = sca.findAsInt('scale factor type').setDiscreteIndex(method)
  if method == ScaleType.UNIFORM:
    # FIXME: We should allow the origin to be specified for this case.
    sca.findAsDouble('scale factor').setValue(factor)
  elif method == ScaleType.PER_AXIS:
    SetVectorValue(sca.findAsDouble('scale factors'), factor)
  if 'origin' in kwargs:
    origin = sca.findAsDouble('origin')
    SetVectorValue(origin, kwargs['origin'])
  res = sca.operate()
  SetLastResult(res)
  PrintResultLog(res)
  return GetVectorValue(res.findModelEntity('modified'))

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
  SetLastResult(res)
  PrintResultLog(res)
  return GetVectorValue(res.findModelEntity('modified'))

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
  SetLastResult(res)
  PrintResultLog(res)
  return res.findModelEntity('created').value(0)

def CreateVertices(pt, model, **kwargs):
  """Create one or more vertices given point coordinates.
  Point coordinates should be specified as a list of 3-tuples.
  The vertices are inserted into the given model
  """
  import itertools
  sref = GetActiveSession()
  # Not all sessions define "create vertices"
  # Fall back to "create vertex" if needed:
  crv = sref.op('create vertices')
  if not crv:
      return [CreateVertex(pt[i]) for i in range(numPts)]
  # OK, we have create vertices.
  # Determine the maximum number of coordinates per point
  numPts = len(pt)
  crv.associateEntity(model)
  numCoordsPerPoint = max([len(pt[i]) for i in range(numPts)])
  pgi = crv.findAsInt('point dimension')
  pgi.setDiscreteIndex(0 if numCoordsPerPoint == 2 else 1)
  pgr = crv.findAsGroup('2d points' if numCoordsPerPoint == 2 else '3d points')
  pgr.setNumberOfGroups(numPts)
  for ix in range(numPts):
      xx = smtk.attribute.to_concrete(pgr.item(ix,0))
      SetVectorValue(xx, pt[ix][0:numCoordsPerPoint] + [0,]*(numCoordsPerPoint - len(pt[ix])))
  res = crv.operate()
  SetLastResult(res)
  PrintResultLog(res)
  created = res.findModelEntity('created')
  return [created.value(i) for i in range(created.numberOfValues())]

def CreateEdge(verts, curve_type = CurveType.LINE, **kwargs):
  """Create an edge from a pair of vertices.
  """
  import itertools
  sref = GetActiveSession()
  cre = sref.op('create edge')
  # Some kernels accept points (not model-verts) in which case
  # the model should be associated with the operator. Otherwise,
  # the model vertices should be associated with the operator.
  if len(verts) < 1:
    print 'Error: No vertices specified.'
    return None
  if hasattr(verts[0], '__iter__'):
    # Verts is actually a list of tuples specifying point coordinates.
    # Look for a model to associate with the operator.
    if 'model' not in kwargs:
      print 'Error: No model specified.'
      return None
    cre.associateEntity(kwargs['model'])
    # Pad and flatten point data
    numCoordsPerPoint = max([len(verts[i]) for i in range(len(verts))])
    tmp = min([len(verts[i]) for i in range(len(verts))])
    x = cre.findAsDouble('points')
    c = cre.findAsInt('coordinates')
    if c:
      c.setValue(0, numCoordsPerPoint)
    if tmp != numCoordsPerPoint:
      ptflat = []
      for p in verts:
        ptflat.append(p + [0,]*(numCoordsPerPoint - len(p)))
      ptflat = list(itertools.chain(*ptflat))
    else:
      ptflat = list(itertools.chain(*verts))
    if x:
      SetVectorValue(x, ptflat)
  else:
    [cre.associateEntity(x) for x in verts]
  t = cre.findAsInt('curve type')
  if t:
    t.setValue(0, curve_type)
  if 'offsets' in kwargs:
    o = cre.findAsInt('offsets')
    if o:
      SetVectorValue(o, kwargs['offsets'])
  if 'midpoint' in kwargs:
    x = cre.findAsDouble('point')
    if x:
      SetVectorValue(x, kwargs['midpoint'])
  if 'color' in kwargs:
    c = cre.findAsInt('color')
    if c:
      c.setValue(0, kwargs['color'])
  res = cre.operate()
  SetLastResult(res)
  PrintResultLog(res)
  entList = res.findModelEntity('created')
  numNewEnts = entList.numberOfValues()
  edgeList = []
  for i in range(numNewEnts):
    if entList.value(i).isEdge():
      edgeList.append(entList.value(i))
  return edgeList[0] if len(edgeList) == 1 else edgeList

def TweakEdge(edge, newPts, **kwargs):
  """Tweak an edge by providing a new set of points along it.
  """
  import itertools
  sref = GetActiveSession()
  twk = sref.op('tweak edge')
  twk.associateEntity(edge)
  numCoordsPerPoint = max([len(newPts[i]) for i in range(len(newPts))])
  tmp = min([len(newPts[i]) for i in range(len(newPts))])
  x = twk.findAsDouble('points')
  c = twk.findAsInt('coordinates')
  if c:
    c.setValue(0, numCoordsPerPoint)
  if tmp != numCoordsPerPoint:
    ptflat = []
    for p in newPts:
      ptflat.append(p + [0,]*(numCoordsPerPoint - len(p)))
    ptflat = list(itertools.chain(*ptflat))
  else:
    ptflat = list(itertools.chain(*newPts))
  if x:
    SetVectorValue(x, ptflat)
  if 'promote' in kwargs:
    o = twk.findAsInt('promote')
    if o:
      SetVectorValue(o, kwargs['promote'])
  res = twk.operate()
  SetLastResult(res)
  PrintResultLog(res)
  modlist = res.findModelEntity('modified')
  result = [modlist.value(i) for i in range(modlist.numberOfValues())]
  crelist = res.findModelEntity('created')
  result += [crelist.value(i) for i in range(crelist.numberOfValues())]
  return result

def SplitEdge(edge, point, **kwargs):
  """Split an edge at a point along the edge.
  """
  import itertools
  sref = GetActiveSession()
  spl = sref.op('split edge')
  spl.associateEntity(edge)
  x = spl.findAsDouble('point')
  SetVectorValue(x, point)
  res = spl.operate()
  SetLastResult(res)
  PrintResultLog(res)
  edgeList = res.findModelEntity('created')
  numEdges = edgeList.numberOfValues()
  return edgeList.value(0) if numEdges == 1 else [edgeList.value(i) for i in range(numEdges)]

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
  SetLastResult(res)
  PrintResultLog(res)
  return res.findModelEntity('created').value(0)

def CreateFaces(modelOrEdges, **kwargs):
  """Create all possible planar faces from a set of edges.
  """
  sref = GetActiveSession()
  crf = sref.op('create faces')

  # Associate model or edges to operator:
  if hasattr(modelOrEdges, '__iter__'):
    [crf.associateEntity(ent) for ent in modelOrEdges]
  else:
    crf.associateEntity(modelOrEdges)

  res = crf.operate()
  SetLastResult(res)
  PrintResultLog(res)

  faceList = res.findModelEntity('created')
  numFaces = faceList.numberOfValues()
  return faceList.value(0) if numFaces == 1 else [faceList.value(i) for i in range(numFaces)]

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
  SetLastResult(res)
  PrintResultLog(res)
  bodies = res.findModelEntity('created')
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
  SetLastResult(res)
  PrintResultLog(res)
  return res.findModelEntity('created').value(0)

def SetEntityProperty(ents, propName, **kwargs):
  """Set a property value (or vector of values) on an entity (or vector of entities).

  You may pass any combination of "as_int", "as_float", or "as_string" as named
  arguments specifying the property values. The values of these named arguments may
  be a single value or a list of values. Values will be coerced to the named type.

  Example:

    SetEntityProperty(face, 'color', as_float=(1., 0., 0.))
    SetEntityProperty(edge, 'name', as_string='edge 20')
    SetEntityProperty(body, 'visited', as_int='edge 20')
  """
  sref = GetActiveSession()
  spr = sref.op('set property')
  if hasattr(ents, '__iter__'):
    [spr.associateEntity(ent) for ent in ents]
  else:
    spr.associateEntity(ents)
  spr.findAsString('name').setValue(propName)
  if 'as_int' in kwargs:
    vlist = kwargs['as_int']
    if not hasattr(vlist, '__iter__'):
      vlist = [vlist,]
    SetVectorValue(spr.findAsInt('integer value'), vlist)
  if 'as_float' in kwargs:
    vlist = kwargs['as_float']
    if not hasattr(vlist, '__iter__'):
      vlist = [vlist,]
    SetVectorValue(spr.findAsDouble('float value'), vlist)
  if 'as_string' in kwargs:
    vlist = kwargs['as_string']
    if not hasattr(vlist, '__iter__'):
      vlist = [vlist,]
    SetVectorValue(spr.findAsString('string value'), vlist)
  res = spr.operate()
  SetLastResult(res)
  return res.findInt('outcome').value(0)

def Read(filename, **kwargs):
  """Read entities from an file (in the native modeling kernel format).
  """
  sref = GetActiveSession()
  rdr = sref.op('read')
  rdr.findAsFile('filename').setValue(0, filename)
  if 'filetype' in kwargs:
    rdr.findAsString('filetype').setValue(0, kwargs['filetype'])
  res = rdr.operate()
  SetLastResult(res)
  PrintResultLog(res)
  return GetVectorValue(res.findModelEntity('created'))

def Import(filename, **kwargs):
  """Import entities from an file (in a non-native modeling kernel format).
  """
  sref = GetActiveSession()
  rdr = sref.op('import')
  # Not all modeling kernels have an import operator:
  if rdr == None:
    return []
  rdr.findAsFile('filename').setValue(0, filename)
  if 'filetype' in kwargs:
    rdr.findAsString('filetype').setValue(0, kwargs['filetype'])
  res = rdr.operate()
  SetLastResult(res)
  PrintResultLog(res)
  return GetVectorValue(res.findModelEntity('created'))

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
  SetLastResult(res)
  PrintResultLog(res)
  return res.findInt('outcome').value(0)

def CloseModel(models = [], **kwargs):
  """Close a set of models using "close model" operator, which invokes "remove model"
  in the session internally if the session has it.
  """
  sref = GetActiveSession()
  closeop = sref.op('close model')
  moditem = closeop.findAsModelEntity('model');
  SetVectorValue(moditem, models)

  res = closeop.operate()
  SetLastResult(res)
  PrintResultLog(res)

  return res

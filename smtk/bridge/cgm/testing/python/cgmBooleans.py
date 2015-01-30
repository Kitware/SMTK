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
sess = mgr.createSession('cgm')
brg = sess.session()
sess.assignDefaultName()
print '\n\n%s: type "%s" %s %s' % \
  (sess.name(), brg.name(), sess.flagSummary(0), brg.sessionId())
print '  Site: %s' % (sess.site() or 'local')

# We could evaluate the session tag as JSON, but most of
# the information is available through methods above that
# we needed to test:
sessiontag = sess.tag()
print '\n'

opnames = sess.operatorNames()

def SetVectorValue(item,v):
  item.setNumberOfValues(len(v))
  for i in range(len(v)):
    item.setValue(i,v[i])

def GetVectorValue(item):
  N = item.numberOfValues()
  return [item.value(i) for i in range(N)]

def GetActiveSession():
  return sess

def CreateSphere(**args):
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
  sph = res.findModelEntity('entities').value(0)
  return sph

def CreateBrick(**args):
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
  brick = res.findModelEntity('entities').value(0)
  return brick

def Intersect(bodies, **args):
  sess = GetActiveSession()
  op = sess.op('intersection')
  try:
    [op.associateEntity(x) for x in bodies]
  except:
    op.associateEntity(bodies)
  res = op.operate()
  return res.findModelEntity('entities').value(0)

def Union(bodies, **args):
  sess = GetActiveSession()
  op = sess.op('union')
  try:
    [op.associateEntity(x) for x in bodies]
  except:
    op.associateEntity(bodies)
  res = op.operate()
  return res.findModelEntity('entities').value(0)

def Subtract(workpiece, tool, **args):
  """Perform a boolean subtraction of the tool from the workpiece.

  Tool and workpiece may each be either an SMTK model or a list of models.
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
  SetVectorValue(op.findAsModel('tools'), tool)

  res = op.operate()
  return res.findModelEntity('entities').value(0)

def Translate(bodies, vec):
  """Translate the body (or list of bodies) along the given vector."""
  top = sess.op('translate')
  if type(bodies) == type([]):
    [top.associateEntity(bod) for bod in bodies]
  else:
    top.associateEntity(bodies)
  SetVectorValue(top.findAsDouble('offset'),vec)
  res = top.operate()
  return GetVectorValue(res.findModelEntity('entities'))

b0 = CreateBrick(center=[0,0,0])
Translate(b0, [0, 1.5, 0])
s0 = CreateSphere(center=[0.5,2.0,0.5], radius=0.5)
bsuni = Union(bodies=[b0, s0])

b0 = CreateBrick(center=[0,0,0])
s0 = CreateSphere(center=[0.5,0.5,0.5], radius=0.5)
bsint = Intersect(bodies=[b0, s0])

b0 = CreateBrick(center=[0,0,0])
s0 = CreateSphere(center=[0.5,0.5,0.5], radius=0.5)
bssub = Subtract(workpiece=b0, tool=s0)

Translate(bsint, [0.1, 0.1, 0.1])

json = smtk.io.ExportJSON.fromModelManager(mgr)
sphFile = open('boolean.json', 'w')
print >> sphFile, json
sphFile.close()

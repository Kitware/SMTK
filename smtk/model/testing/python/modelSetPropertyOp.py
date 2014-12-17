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
Try running a "universal" operator on an imported model.
"""

import os
import smtk
from uuid import uuid4

import sys
model_path = os.path.join(sys.argv[1], 'smtk', 'pyramid.json')
print 'Loading %s' % model_path

status = 0
mgr = smtk.model.Manager.create()
session = mgr.createAndRegisterBridge('native', uuid4())
json = None
with open(model_path, 'r') as f:
  json = f.read()
if json is None:
  logging.error('Unable to load input file')
  sys.exit(2)
if not smtk.io.ImportJSON.intoModel(json, mgr):
  logging.error('Uname to parse json input file')
  sys.exit(4)
mgr.assignDefaultNames()
models = mgr.findEntitiesOfType(smtk.model.MODEL_ENTITY, True)
print 'Applying operator to %d model(s)' % len(models)

op = smtk.model.ModelEntity(models[0]).op('set property')
op.findAsString('name').setValue('superduperness')
op.findAsInt('integer value').setNumberOfValues(1)
op.findAsInt('integer value').setValue(42)
[op.associateEntity(x) for x in models]
result = op.operate()
if result.findInt('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
  print 'Set property operator failed'
  sys.exit(8)
print 'Checking properties'
for x in mgr.findEntitiesOfType(smtk.model.ANY_ENTITY, False):
  sdness = x.integerProperty('superduperness')
  if not (not x.isModelEntity()) ^ (len(sdness) == 1 and sdness[0] == 42):
    print x.name(), ' has unexpected superduperness of ', sdness
    status = 1

print 'Done'

if __name__ == '__main__':
  sys.exit(status)

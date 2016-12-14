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
import sys
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.io
    import smtk.model
import smtk.testing
from uuid import uuid4
import unittest

class TestModelSetPropertyOp(unittest.TestCase):

  def testSetPropertyOp(self):
    model_path = os.path.join(smtk.testing.DATA_DIR, 'model', '2d', 'smtk', 'pyramid.json')
    print 'Loading %s' % model_path

    status = 0
    mgr = smtk.model.Manager.create()
    session = mgr.createSession('native', smtk.model.SessionRef())
    json = None
    with open(model_path, 'r') as f:
      json = f.read()

    self.assertTrue(not json == None, 'Unable to load input file')
    self.assertTrue(smtk.io.ImportJSON.intoModelManager(json, mgr), 'Unable to parse JSON input file')

    mgr.assignDefaultNames()
    models = mgr.findEntitiesOfType(int(smtk.model.MODEL_ENTITY), True)
    # Assign imported models to current session so they have operators
    [smtk.model.Model(x).setSession(session) for x in models]
    print 'Applying operator to %d model(s)' % len(models)

    op = smtk.model.Model(models[0]).op('set property')
    op.findAsString('name').setValue('superduperness')
    op.findAsInt('integer value').setNumberOfValues(1)
    op.findAsInt('integer value').setValue(42)
    [op.associateEntity(x) for x in models]
    result = op.operate()
    self.assertEqual(
        result.findInt('outcome').value(0),
        smtk.model.OPERATION_SUCCEEDED,
        'Set property operator failed')
    print 'Checking properties'
    for x in mgr.findEntitiesOfType(int(smtk.model.ANY_ENTITY), False):
      sdness = x.integerProperty('superduperness')
      if not (not x.isModel()) ^ (len(sdness) == 1 and sdness[0] == 42):
        print x.name(), ' has unexpected superduperness of ', sdness
        status = 1

    print 'Done'

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()

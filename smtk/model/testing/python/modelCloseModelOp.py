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
import smtk.testing
from uuid import uuid4
import unittest

class TestModelCloseModelOp(unittest.TestCase):

  def testCloseModelOp(self):
    model_path = os.path.join(smtk.testing.DATA_DIR, 'smtk', 'pyramid.json')
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
    models = mgr.findEntitiesOfType(smtk.model.MODEL_ENTITY, True)
    # Assign imported models to current session so they have operators
    [smtk.model.Model(x).setSession(session) for x in models]
    print 'Applying operator to %d model(s)' % len(models)

    op = smtk.model.Model(models[0]).op('close model')
    op.findAsModelEntity('model').setNumberOfValues(1)
    op.findAsModelEntity('model').setValue(models[0])

    result = op.operate()
    self.assertEqual(
        result.findInt('outcome').value(0),
        smtk.model.OPERATION_SUCCEEDED,
        'close model operator failed')
    print 'Checking models'
    for x in mgr.findEntitiesOfType(smtk.model.MODEL_ENTITY, True):
      if x.entity().toString() == models[0].entity().toString():
        print 'Closing %s has failed ' % x.name()
        status = 1
        break

    print 'Done'

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()

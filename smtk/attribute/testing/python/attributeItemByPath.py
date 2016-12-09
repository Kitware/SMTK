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
from smtk import attribute
from smtk import io

from smtk.simple import *
import smtk.testing
import os

class TestAttributeItemByPath(smtk.testing.TestCase):

    def setUp(self):
        if smtk.testing.DATA_DIR == '':
          self.skipTest('SMTK test-data directory not provided')

        self.system = smtk.attribute.System()
        logger = smtk.io.Logger()
        reader = smtk.io.AttributeReader()
        filenm = os.path.join(smtk.testing.DATA_DIR, 'attribute', 'attribute_system', 'HydraTemplateV1.sbt')
        status = reader.read(self.system, filenm, logger)
        print '\n'.join([logger.record(i).message for i in range(logger.numberOfRecords())])
        self.assertFalse(status, 'Could not read {fn}'.format(fn=filenm))

    def testItemInSimplePath(self):
        att = self.system.createAttribute('hydrostat')
        itemInSimplePath = att.itemAtPath('Hydrostat', '/')
        self.assertIsNotNone(itemInSimplePath, 'Could not find expected item.')
        self.assertEqual(itemInSimplePath.name(), 'Hydrostat', 'Got wrong attribute "{nm}".'.format(nm=itemInSimplePath.name()))

    def testItemInChildPath(self):
        att = self.system.createAttribute('BasicTurbulenceModel')
        itemInChildren = att.itemAtPath('Method/prandtl', '/')
        self.assertIsNotNone(itemInChildren, 'Could not find expected item.')
        self.assertEqual(itemInChildren.name(), 'prandtl', 'Got wrong attribute "{nm}".'.format(nm=itemInChildren.name()))

    def testItemInGroupPath(self):
        att = self.system.createAttribute('InitialConditions')
        itemInGroup = att.itemAtPath('InitialConditions/Velocity', '/')
        self.assertIsNotNone(itemInGroup, 'Could not find expected item.')
        self.assertEqual(itemInGroup.name(), 'Velocity', 'Got wrong attribute "{nm}".'.format(nm=itemInGroup.name()))

    def testItemInDeepPath(self):
        att = self.system.createAttribute('ppesolver')
        deepItem = att.itemAtPath('PressurePoissonSolver/ppetype/preconditioner', '/')
        self.assertIsNotNone(deepItem, 'Could not find expected item.')
        self.assertEqual(deepItem.name(), 'preconditioner', 'Got wrong attribute "{nm}".'.format(nm=deepItem.name()))

        # Do the same thing with a different separator
        deepItem = att.itemAtPath('PressurePoissonSolver-ppetype-preconditioner', '-')
        self.assertIsNotNone(deepItem, 'Could not find expected item with alternate path separator.')
        self.assertEqual(deepItem.name(), 'preconditioner',
            'Got wrong attribute "{nm}" with alt. path separator.'.format(nm=deepItem.name()))

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()

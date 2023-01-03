import sys
# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================
import smtk
from smtk import attribute
from smtk import io

import smtk.testing
import os


class TestAttributeQuery(smtk.testing.TestCase):

    def setUp(self):
        if smtk.testing.DATA_DIR == '':
            self.skipTest('SMTK test-data directory not provided')

        self.resource = smtk.attribute.Resource.create()
        logger = smtk.io.Logger()
        reader = smtk.io.AttributeReader()
        filenm = os.path.join(smtk.testing.DATA_DIR, 'attribute',
                              'attribute_collection', 'HydraTemplateV1.sbt')
        status = reader.read(self.resource, filenm, logger)
        print(
            '\n'.join([logger.record(i).message for i in range(logger.numberOfRecords())]))
        self.assertFalse(status, 'Could not read {fn}'.format(fn=filenm))

        att = self.resource.createAttribute('hydrostat')
        att = self.resource.createAttribute('BasicTurbulenceModel')
        att = self.resource.createAttribute('InitialConditions')
        att = self.resource.createAttribute('ppesolver')

    def testQueryAny(self):
        attrs = self.resource.filter('any')
        for aa in attrs:
            print('Found %s of type %s' % (aa.name(), aa.type()))
        self.assertEqual(
            len(attrs), 4, 'Expected 4 attributes, got %d.' % len(attrs))

    def testQueryByDefinition(self):
        attrs = self.resource.filter("attribute[type='hydrostat']")
        self.assertEqual(
            len(attrs), 1, 'Expected 1 attributes, got %d.' % len(attrs))
        attr = attrs.pop()
        self.assertEqual(attr.type(), 'hydrostat',
                         'Unexpected attribute type "%s"' % attr.type())


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()

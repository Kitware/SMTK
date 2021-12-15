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

import os
import sys
import unittest

import smtk
import smtk.attribute
import smtk.io
import smtk.testing

import smtk.attribute_builder

SBT = """
<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <String Name="string-item" />
        <Double Name="double-item" />
        <Int Name="int-item" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
"""

SPEC = {
    'items': [
        {'name': 'string-item', 'value': 'xyzzy'},
        {'name': 'double-item', 'value': 3.14159},
        {'name': 'int-item', 'value': 42},
    ]
}


class TestConfigureAttribute(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_configure_attribute(self):
        """"""
        resource = smtk.attribute.Resource.New()

        # Load attribute template
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        # sbt_path = os.path.join(smtk.testing.DATA_DIR, 'example.sbt')
        # err = reader.read(resource, sbt_path, logger)
        err = reader.readContents(resource, SBT, logger)
        self.assertFalse(err, 'error loading attribute template')

        # Create attribute
        defn = resource.findDefinition('Example')
        self.assertIsNotNone(defn, 'failed to find Example definition')
        att = resource.createAttribute('test', defn)
        self.assertIsNotNone(att, 'failed to create test attribute')

        # Create builder and apply config spec (dictionary)
        builder = smtk.attribute_builder.AttributeBuilder()
        builder._edit_attribute(att, SPEC)

        # Check
        sitem = att.findString('string-item')
        self.assertEqual(sitem.value(), 'xyzzy', 'failed to set string item')

        ditem = att.findDouble('double-item')
        self.assertAlmostEqual(ditem.value(), 3.14159,
                               'failed to set double item')

        iitem = att.findInt('int-item')
        self.assertEqual(iitem.value(), 42, 'failed to set int-item')

        self.assertTrue(att.isValid(), 'attribute not valid - somehting ng')

        # print('SUCCESS') # sanity check that the test really executed


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

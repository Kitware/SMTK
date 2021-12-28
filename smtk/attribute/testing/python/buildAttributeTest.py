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
        <Double Name="double-item" Extensible="true" NumberOfRequiredValues="1">
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Int Name="int-item" Optional="True" IsEnabledByDefault="False">
          <ChildrenDefinitions>
            <Double Name="conditional-double" />
          </ChildrenDefinitions>
          <DiscreteInfo>
            <Value Enum="first">1</Value>
            <Structure>
              <Value Enum="second">2</Value>
              <Items>
                <Item>conditional-double</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
        <Group Name="group-item">
          <ItemDefinitions>
            <Double Name="subgroup-double" />
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
"""

PATH = 'path'
VALUE = 'value'
SPEC = {
    'items': [
        {PATH: 'string-item', VALUE: 'xyzzy'},
        {PATH: '/double-item', VALUE: [3.14159, None, 2.71828]},
        {PATH: 'int-item', 'enabled': True, VALUE: 2},
        {PATH: 'int-item/conditional-double', VALUE: 42.42},
        {PATH: '/group-item/subgroup-double', VALUE: 73.73},
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

        # Write result (for debug)
        writer = smtk.io.AttributeWriter()
        filepath = os.path.join(smtk.testing.TEMP_DIR, 'config.sbi')
        err = writer.write(resource, filepath, logger)
        self.assertFalse(err, 'Error writing file {}'.format(filepath))
        print('Wrote', filepath)

        # Check
        sitem = att.findString('string-item')
        self.assertEqual(sitem.value(), 'xyzzy', 'failed to set string item')

        ditem = att.findDouble('double-item')
        self.assertAlmostEqual(ditem.value(0), 3.14159,
                               'failed to set double item first value')
        self.assertFalse(ditem.isSet(
            1), 'failed to unset double item second value')
        self.assertAlmostEqual(ditem.value(2), 2.71828,
                               'failed to set double item third value')

        iitem = att.findInt('int-item')
        self.assertTrue(iitem.isEnabled(), 'failed to enable int-item')
        self.assertEqual(iitem.value(), 2, 'failed to set int-item')

        cond_item = iitem.find('conditional-double')
        self.assertAlmostEqual(cond_item.value(), 42.42)

        group_item = att.findGroup('group-item')
        subgroup_item = group_item.find('subgroup-double')
        self.assertAlmostEqual(subgroup_item.value(), 73.73)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

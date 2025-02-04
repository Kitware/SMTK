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
import smtk.session.vtk
import smtk.testing

import smtk.attribute_builder

OP_SUCCEEDED = int(smtk.operation.Operation.Outcome.SUCCEEDED)

SBT = """
<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="Example">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <Accepts>
          <Resource Name="smtk::resource::Resource" />
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="string-item" />
        <Double Name="double-item"  NumberOfRequiredValues="3">
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
        <Component Name="comp-item" Extensible="true">
          <Accepts>
            <!-- Note: Filter is specific to smtk::model::Resource -->
            <Resource Name="smtk::resource::Resource" Filter="volume|face" />
          </Accepts>
        </Component>
        <Group Name="group-item" Extensible="true">
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
    'associations': [{'resource': 'model'}],
    'items': [
        {PATH: 'string-item', VALUE: 'xyzzy'},
        {PATH: '/double-item', VALUE: [3.14159, None, 2.71828]},
        {PATH: 'int-item', 'enabled': True, VALUE: 2},
        {PATH: 'int-item/conditional-double', VALUE: 42.42},
        {PATH: 'comp-item', VALUE: [
            {'resource': 'model', 'component': 'casting'},
            {'resource': 'model', 'component': 'symmetry'},
        ]},
        {PATH: '/group-item', 'count': 2},
        {PATH: '/group-item/0/subgroup-double', VALUE: 73.73},
        {PATH: '/group-item/1/subgroup-double', VALUE: 83.83},
    ]
}


class TestBuildAttribute(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def import_attribute_template(self):
        """Instantiates attribute resource"""
        resource = smtk.attribute.Resource.New()

        # Load attribute template
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        # sbt_path = os.path.join(smtk.testing.DATA_DIR, 'example.sbt')
        # err = reader.read(resource, sbt_path, logger)
        err = reader.readContents(resource, SBT, logger)
        self.assertFalse(err, 'error loading attribute template')
        return resource

    def import_model(self):
        """"""
        gen_path = os.path.join(smtk.testing.DATA_DIR,
                                'model/3d/genesis/filling1.gen')
        import_op = smtk.session.vtk.Import.create()
        import_op.parameters().findFile('filename').setValue(gen_path)
        result = import_op.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, OP_SUCCEEDED,
                         'Failed to import model file {}'.format(gen_path))
        resource = smtk.model.Resource.CastTo(
            result.find('resourcesCreated').value())

        return resource

    def test_configure_attribute(self):
        """"""
        logger = smtk.io.Logger.instance()

        # Load resources
        att_resource = self.import_attribute_template()
        model_resource = self.import_model()

        # Create test attribute
        defn = att_resource.findDefinition('Example')
        self.assertIsNotNone(defn, 'failed to find Example definition')
        att = att_resource.createAttribute('test', defn)
        self.assertIsNotNone(att, 'failed to create test attribute')

        # Create builder and apply config spec (dictionary)
        builder = smtk.attribute_builder.AttributeBuilder()
        resource_dict = dict(model=model_resource)
        builder.build_attribute(att, SPEC, resource_dict=resource_dict)

        # Write result (for debug)
        writer = smtk.io.AttributeWriter()
        filepath = os.path.join(smtk.testing.TEMP_DIR, 'config.sbi')
        err = writer.write(att_resource, filepath, logger)
        self.assertFalse(err, 'Error writing file {}'.format(filepath))
        print('Wrote', filepath)

        # Check
        assocs = att.associations()
        self.assertEqual(assocs.numberOfValues(), 1,
                         'missing resource association')
        self.assertTrue(att.isObjectAssociated(model_resource))

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

        comp_item = att.findComponent('comp-item')
        self.assertEqual(comp_item.numberOfValues(), 2,
                         'wrong number of comp-item values, should be 2 found {}'.format(comp_item.numberOfValues()))
        self.assertIsNotNone(comp_item.value(
            0), 'first component item value not set')
        self.assertIsNotNone(comp_item.value(
            1), 'second component item value not set')

        group_item = att.findGroup('group-item')
        subgroup_item = group_item.find('subgroup-double')
        self.assertAlmostEqual(subgroup_item.value(), 73.73)
        subgroup_item = group_item.find(1, 'subgroup-double')
        self.assertAlmostEqual(subgroup_item.value(), 83.83)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

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
from smtk import attribute
from smtk import io
from smtk import operation

import smtk.testing

OPERATION_SUCCEEDED = 3


template1 = """
<SMTK_AttributeResource Version="3">
    <Definitions>
        <AttDef Type="Test1">
            <ItemDefinitions>
                <Group Name="group0" Extensible="true" NumberOfRequiredGroups="0">
                    <ItemDefinitions>
                        <String Name="StringItem" />
                    </ItemDefinitions>
                </Group>
                <Group Name="group1" Extensible="true" NumberOfRequiredGroups="1">
                    <ItemDefinitions>
                        <String Name="StringItem" />
                    </ItemDefinitions>
                </Group>
                <Group Name="group2" Extensible="true" NumberOfRequiredGroups="1">
                    <ItemDefinitions>
                        <String Name="StringItem" />
                    </ItemDefinitions>
                </Group>
            </ItemDefinitions>
        </AttDef>
    </Definitions>
</SMTK_AttributeResource>
"""


class TestExtensibleGroup(smtk.testing.TestCase):
    """Test for GroupItem I/O


    Detects an I/O error for group items with these characteristics
      * extensible
      * number of required values 1
      * actual number of values > 1
    """

    def setUp(self):
        # Remember files created during test
        self.filenames = list()

        # Initialize operation manager
        self.op_manager = smtk.operation.Manager.create()
        smtk.attribute.Registrar.registerTo(self.op_manager)
        smtk.operation.Registrar.registerTo(self.op_manager)

    def tearDown(self):
        for path in self.filenames:
            if os.path.exists(path):
                os.remove(path)

    def init_source_resource(self):
        """"""
        logger = smtk.io.Logger.instance()

        # Parse template
        reader = smtk.io.AttributeReader()
        resource = smtk.attribute.Resource.create()
        err = reader.readContents(resource, template1, logger)
        self.assertFalse(err, 'Failed to parse attribute template1')

        # Create attribute and edit group items
        defn = resource.findDefinition('Test1')
        att = resource.createAttribute('Test1', defn)
        group2 = att.findGroup('group2')
        string0 = group2.item(0, 0)
        string0.setValue('string000')
        group2.appendGroup()
        string1 = group2.item(1, 0)
        string1.setValue('string111')

        return resource

    def write_read_xml(self, resource, filename):
        """Write xml format and read back"""
        logger = smtk.io.Logger.instance()
        path = os.path.join(smtk.testing.TEMP_DIR, filename)
        self.filenames.append(path)

        # Write as xml
        writer = smtk.io.AttributeWriter()
        err = writer.write(resource, path, logger)
        self.assertFalse(err, 'Failed to write attribute template1')

        # Read back
        test_resource = smtk.attribute.Resource.create()
        reader = smtk.io.AttributeReader()
        err = reader.read(test_resource, path, logger)
        self.assertFalse(err, 'Failed to read attribute template1')

        return test_resource

    def write_read_json(self, resource, filename):
        """Write smtk format and read back"""
        path = os.path.join(smtk.testing.TEMP_DIR, filename)
        self.filenames.append(path)
        resource.setLocation(path)

        # Write as smtk
        write_op = self.op_manager.createOperation(
            'smtk::operation::WriteResource')
        write_params = write_op.parameters()
        write_params.associate(resource)
        write_result = write_op.operate()

        # Read back
        read_op = self.op_manager.createOperation(
            'smtk::operation::ReadResource')
        read_params = read_op.parameters()
        file_item = read_params.findFile('filename')
        file_item.setValue(path)
        read_result = read_op.operate()

        # Also check log for errors
        if read_op.log().hasErrors():
            print('READ LOG:')
            print(read_op.log().convertToString())
        self.assertFalse(read_op.log().hasErrors())

        resource_item = read_result.findResource("resourcesCreated")
        test_resource = resource_item.value()
        self.assertIsNotNone(test_resource)
        return test_resource

    def verify(self, resource):
        """"""
        att = resource.findAttribute('Test1')
        self.assertIsNotNone(att)

        group2 = att.findGroup('group2')
        self.assertIsNotNone(group2)
        self.assertEqual(group2.numberOfGroups(), 2)

        # Sanity check first subgroup
        string0 = group2.item(0, 0)
        self.assertTrue(string0)
        self.assertEqual(string0.value(), 'string000')

        # Check second subgroup
        string1 = group2.item(1, 0)
        self.assertIsNotNone(string1)
        self.assertTrue(string1)
        self.assertEqual(string1.value(), 'string111')

    def test_xml(self):
        source_resource = self.init_source_resource()
        xml_resource = self.write_read_xml(
            source_resource, 'extensibleGroupTest.sbi')
        self.verify(xml_resource)

    def test_json(self):
        source_resource = self.init_source_resource()
        json_resource = self.write_read_json(
            source_resource, 'extensibleGroupTest.smtk')
        self.verify(json_resource)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()

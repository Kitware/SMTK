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

template1 = """
<SMTK_AttributeResource Version="3">
    <Definitions>
        <AttDef Type="Test1" BaseType="">
            <ItemDefinitions>
                <Group Name="Group1" Extensible="true" NumberOfRequiredGroups="0">
                    <ItemDefinitions>
                        <Int Name="IntItem" />
                        <String Name="StringItem" />
                    </ItemDefinitions>
                </Group>
            </ItemDefinitions>
        </AttDef>
    </Definitions>
</SMTK_AttributeResource>
"""


class TestGroupItemRotate(smtk.testing.TestCase):

    def setUp(self):
        # Read template1
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        self.att_resource = smtk.attribute.Resource.create()
        err = reader.readContents(self.att_resource, template1, logger)
        self.assertFalse(err, 'Failed to read attribute template1')

    def tearDown(self):
        att = self.att_resource.findAttribute('Test1')
        if att is not None:
            self.att_resource.removeAttribute(att)
        self.att_resource = None

    def init_group_item(self, length, factor=1):
        """Create attribute and populate group item

        length: number of subgroups to create
        factor: scale factor to use setting int value
        return: group_item
        """
        defn = self.att_resource.findDefinition('Test1')
        att = self.att_resource.createAttribute('Test1', defn)
        group_item = att.findGroup('Group1')

        for i in range(length):
            group_item.appendGroup()
            int_item = group_item.item(i, 0)
            int_item.setValue(i * factor)

            string_item = group_item.item(i, 1)
            string_item.setValue('s{}'.format(i))
        return group_item

    def get_values(self, group_item, ith=0):
        """Retrieves values from groups.

        Only works if GroupItem is storing ValueItem instances
        """
        num_groups = group_item.numberOfGroups()
        vals = [None] * num_groups
        for i in range(num_groups):
            vals[i] = group_item.item(i, ith).value()
        return vals

    def check_ints(self, expected_values, group_item, ith=0):
        """Compares expected values with contents of group_item"""
        actual_values = self.get_values(group_item, ith)
        self.assertEqual(len(expected_values), len(actual_values))

        diff = sum([1 for i in range(len(actual_values))
                    if actual_values[i] != expected_values[i]])
        self.assertEqual(diff, 0)

    def check_string(self, expected_value, group_item, ith=1):
        """Checks expected string with contents of group item"""
        actual_value = group_item.item(ith, 1).value()
        self.assertEqual(expected_value, actual_value)

    # Tests 1-3 rotate forward (fromPosition < toPosition)
    def test1(self):
        group_item = self.init_group_item(6, 11)
        rotated = group_item.rotate(2, 4)
        self.assertTrue(rotated)
        vals = self.get_values(group_item)
        print(vals)
        self.check_ints([0, 11, 33, 44, 22, 55], group_item)
        self.check_string('s3', group_item, 2)
        self.check_string('s2', group_item, 4)

    def test2(self):
        group_item = self.init_group_item(7, 11)
        rotated = group_item.rotate(0, 4)  # rotate 1st subgroup
        self.assertTrue(rotated)
        vals = self.get_values(group_item)
        print(vals)
        self.check_ints([11, 22, 33, 44, 0, 55, 66], group_item)
        self.check_string('s0', group_item, 4)

    def test3(self):
        group_item = self.init_group_item(5, 11)
        rotated = group_item.rotate(3, 4)  # rotate to last subgroup
        self.assertTrue(rotated)
        vals = self.get_values(group_item)
        print(vals)
        self.check_ints([0, 11, 22, 44, 33], group_item)
        self.check_string('s3', group_item, 4)

    # Tests 4-6 rotate backward (fromPosition > toPosition)
    def test4(self):
        group_item = self.init_group_item(6)
        rotated = group_item.rotate(4, 2)
        self.assertTrue(rotated)
        vals = self.get_values(group_item)
        print(vals)
        self.check_ints([0, 1, 4, 2, 3, 5], group_item)
        self.check_string('s4', group_item, 2)
        self.check_string('s2', group_item, 3)

    def test5(self):
        group_item = self.init_group_item(7)
        rotated = group_item.rotate(5, 0)  # rotate to first subgroup
        self.assertTrue(rotated)
        vals = self.get_values(group_item)
        print(vals)
        self.check_ints([5, 0, 1, 2, 3, 4, 6], group_item)
        self.check_string('s5', group_item, 0)

    def test6(self):
        group_item = self.init_group_item(11)
        rotated = group_item.rotate(10, 3)  # rotate last subgroup
        self.assertTrue(rotated)
        vals = self.get_values(group_item)
        print(vals)
        self.check_ints([0, 1, 2, 10, 3, 4, 5, 6, 7, 8, 9], group_item)
        self.check_string('s10', group_item, 3)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()

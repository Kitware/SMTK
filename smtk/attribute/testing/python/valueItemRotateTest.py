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
import sys
import unittest

import smtk
from smtk import attribute
from smtk import io

import smtk.testing

DEFAULT_PRINT = True

template1 = """
<SMTK_AttributeResource Version="3">
    <Definitions>
        <AttDef Type="Expression1" />
        <AttDef Type="Test1">
            <ItemDefinitions>
                <Double Name="DoubleItem1" Extensible="true"/>
                <Int Name="IntItem1" Extensible="true">
                    <ExpressionType>Expression1</ExpressionType>
                </Int>
                <Int Name="DiscreteInt" Extensible="true">
                    <DiscreteInfo>
                        <Value>0</Value>
                        <Value>73</Value>
                        <Value>99</Value>
                        <Value>42</Value>
                    </DiscreteInfo>
                </Int>
                <String Name="StringItem1" Extensible="true"/>
            </ItemDefinitions>
        </AttDef>
    </Definitions>
</SMTK_AttributeResource>
"""


class TestValueItemRotate(smtk.testing.TestCase):

    def setUp(self):
        # Read template1
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        self.att_resource = smtk.attribute.Resource.create()
        err = reader.readContents(self.att_resource, template1, logger)
        self.assertFalse(err, 'Failed to read attribute template1')

        # Create test attribute
        test_defn = self.att_resource.findDefinition('Test1')
        self.att1 = self.att_resource.createAttribute('Test1', test_defn)

        # Create attribute to use as expression
        exp_defn = self.att_resource.findDefinition('Expression1')
        self.exp_att = self.att_resource.createAttribute(
            'Expression1', exp_defn)

    def tearDown(self):
        if self.att1 is not None:
            self.att_resource.removeAttribute(self.att1)
        self.att_resource = None

    def init_value_item(self, value_item, values):
        """Copies list of values into value item

        Caller responsible for passing compatible data
        """
        value_item.setNumberOfValues(len(values))
        for i, val in enumerate(values):
            if val is not None:
                value_item.setValue(i, val)

    def init_int_item(self, length, factor=1):
        """Populate int item

        length: number of values to set
        factor: scale factor to use setting int value
        return: IntItem
        """
        int_item = self.att1.findInt('IntItem1')
        values = [i * factor for i in range(length)]
        self.init_value_item(int_item, values)
        return int_item

    def init_discrete_int(self, indices):
        """Initalizes discrete integer item

        indices: list of index values to assign; list can include None
        """
        int_item = self.att1.findInt('DiscreteInt')
        length = len(indices)
        int_item.setNumberOfValues(length)
        for i in range(length):
            index = indices[i]
            if index is not None:
                int_item.setDiscreteIndex(i, index)
        return int_item

    def get_values(self, value_item):
        """Retrieves values from value item"""
        num_vals = value_item.numberOfValues()
        vals = [None] * num_vals
        for i in range(num_vals):
            if not value_item.isSet(i):
                vals[i] = 'unset'
            elif value_item.isDiscrete():
                vals[i] = value_item.discreteIndex(i)
            elif value_item.isExpression(i):
                vals[i] = 'exp'
            else:
                vals[i] = value_item.value(i)
        return vals

    def check_vals(self, expected_values, value_item, _print=DEFAULT_PRINT):
        """Compares expected values with contents of value_item"""
        actual_values = self.get_values(value_item)
        if _print:
            print(actual_values)
        self.assertEqual(len(expected_values), len(actual_values))

        diff = sum([1 for i in range(len(actual_values))
                    if actual_values[i] != expected_values[i]])
        self.assertEqual(diff, 0)

    def check_double_vals(self, expected_values, value_item, tolerance=1e-6, _print=DEFAULT_PRINT):
        """Compares expected values with contents of value_item"""
        actual_values = self.get_values(value_item)
        if _print:
            print(actual_values)
        self.assertEqual(len(expected_values), len(actual_values))

        diff_count = 0
        for i in range(len(actual_values)):
            if abs(actual_values[i] - expected_values[i]) > tolerance:
                diff_count += 1
        self.assertEqual(diff_count, 0)

    def test_invalid_positions(self):
        int_length = 7
        int_item = self.init_int_item(int_length)
        self.assertFalse(int_item.rotate(9, 5))
        self.assertFalse(int_item.rotate(0, 7))
        self.assertFalse(int_item.rotate(3, 3))

    def test_int_values(self):
        """Test item values & expressions"""
        int_length = 9
        int_factor = 11
        int_item = self.init_int_item(int_length, int_factor)
        self.assertTrue(int_item.rotate(2, 5))  # forward
        self.check_vals([0, 11, 33, 44, 55, 22, 66, 77, 88], int_item)

        self.assertTrue(int_item.rotate(7, 0))  # reverse
        self.check_vals([77, 0, 11, 33, 44, 55, 22, 66, 88], int_item)

        # Unset values at positions 2, 4, 5
        int_item.unset(2)
        int_item.unset(4)
        int_item.unset(5)
        self.check_vals([77, 0, 'unset', 33, 'unset',
                         'unset', 22, 66, 88], int_item)

        rotate = int_item.rotate(2, 8)
        self.check_vals([77, 0, 33, 'unset', 'unset',
                         22, 66, 88, 'unset'], int_item)

        int_item.setValue(3, 44)
        int_item.setValue(8, 11)
        self.check_vals([77, 0, 33, 44, 'unset', 22, 66, 88, 11], int_item)

        # Set expressions at 0, 4
        int_item.setExpression(0, self.exp_att)
        int_item.setExpression(5, self.exp_att)
        self.check_vals(['exp', 0, 33, 44, 'unset',
                         'exp', 66, 88, 11], int_item)

        int_item.rotate(7, 0)
        self.check_vals(
            [88, 'exp', 0, 33, 44, 'unset', 'exp', 66, 11], int_item)

    def test_int_discretes(self):
        """Test discrete item indices"""
        indices = [0, 3, None, 0, 2, 0, 1, None]
        int_item = self.init_discrete_int(indices)
        self.check_vals([0, 3, 'unset', 0, 2, 0, 1, 'unset'], int_item)

        self.assertTrue(int_item.rotate(1, 4))  # forward
        self.check_vals([0, 'unset', 0, 2, 3, 0, 1, 'unset'], int_item)

        self.assertTrue(int_item.rotate(6, 1))  # reverse

        self.assertTrue(int_item.rotate(2, 6))  # unset index
        self.check_vals([0, 1, 0, 2, 3, 0, 'unset', 'unset'], int_item)

        int_item.setDiscreteIndex(6, 3)
        self.check_vals([0, 1, 0, 2, 3, 0, 3, 'unset'], int_item)

    def test_string_values(self):
        """Spot check some cases StringItem"""
        values = ['cero', 'uno', None, 'tres', 'cuatro', 'cinco', 'sies']
        string_item = self.att1.findString('StringItem1')
        string_item.setNumberOfValues(len(values))
        for i, val in enumerate(values):
            if val is not None:
                string_item.setValue(i, val)
        expected = values[:]
        expected[2] = 'unset'
        self.check_vals(expected, string_item)

        self.assertTrue(string_item.rotate(1, 5))
        self.check_vals(
            ['cero', 'unset', 'tres', 'cuatro', 'cinco', 'uno', 'sies'], string_item)

        self.assertTrue(string_item.rotate(4, 0))
        self.check_vals(
            ['cinco', 'cero', 'unset', 'tres', 'cuatro', 'uno', 'sies'], string_item)

    def test_double_values(self):
        """Spot check some cases with DoubleItem"""
        values = [0.0, 1.1, 2.71828, 3.14159, 4.4]
        double_item = self.att1.findDouble('DoubleItem1')
        self.init_value_item(double_item, values)
        print(values)

        self.assertTrue(double_item.rotate(0, 3))
        self.check_double_vals([1.1, 2.71828, 3.14159, 0.0, 4.4], double_item)

        self.assertTrue(double_item.rotate(4, 1))
        self.check_double_vals([1.1, 4.4, 2.71828, 3.14159, 0.0], double_item)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()

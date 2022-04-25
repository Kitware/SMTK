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

import unittest
import smtk
import smtk.attribute
import smtk.attribute_builder
import smtk.extension.paraview.appcomponents
import smtk.io
import smtk.operation
import smtk.session.vtk
import smtk.resource
import smtk.testing
import re
from smtk.operation import configureAttribute
from smtk.extension.paraview.appcomponents import pqSMTKPythonTrace

OP_SUCCEEDED = int(smtk.operation.Operation.Outcome.SUCCEEDED)

SBT = """
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="operation" Label="operation" Abstract="True">
      <ItemDefinitions>
        <Int Name="debug level" Optional="True">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="result" Abstract="True">
      <ItemDefinitions>
        <Int Name="outcome" Label="outcome" Optional="False" NumberOfRequiredValues="1">
        </Int>
        <String Name="log" Optional="True" NumberOfRequiredValues="0" Extensible="True">
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="TestOp" Label="My Test Operation" BaseType="operation">
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
    <AttDef Type="result(test op)" BaseType="result">
      <ItemDefinitions>
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


class TestOp(smtk.operation.Operation):

    def __init__(self):
        smtk.operation.Operation.__init__(self)

    def name(self):
        return "test op"

    def operateInternal(self):
        smtk.WarningMessage(self.log(), 'My string is \"%s\"' %
                            self.parameters().findString('string-item').value())

        # Return with success
        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)
        # result.findString('my string').setValue(
        #     self.parameters().findString('my string').value())
        return result

    def createSpecification(self):
        spec = smtk.attribute.Resource.New()
        reader = smtk.io.AttributeReader()
        err = reader.readContents(spec, SBT, self.log())

        if err:
            print(self.log().convertToString())

        return spec


class TestOperationTracing(smtk.testing.TestCase):
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
        resource = smtk.model.Resource.CastTo(result.find('resource').value())

        return resource

    def test_operation_tracing(self):
        model_resource = self.import_model()

        op = TestOp.create(__name__, "TestOp", 1)
        parameters = op.parameters()

        builder = smtk.attribute_builder.AttributeBuilder()
        resource_dict = dict(model=model_resource)
        SPEC['resources'] = resource_dict
        # use the method that will be produced by python tracing.
        configureAttribute(parameters, SPEC)
        # check it did something.
        assert(parameters.find('int-item').value() == 2)

        # now check that we are able to trace the operation
        tracer = pqSMTKPythonTrace()
        op_trace = tracer.traceOperation(op, testing=True)
        print("operation trace:\n", op_trace)
        # non-default items should be traced.
        assert(op_trace.find(
            "'path': 'double-item', 'value': [ 3.1415899999999999, None, 2.71828 ]") > 0)
        assert(op_trace.find(
            "'path': 'int-item/conditional-double', 'value': 42.42") > 0)
        assert(op_trace.find("'resource': 'filling1', 'component': 'casting'") > 0)
        assert(op_trace.find("'resource': 'filling1', 'component': 'symmetry'") > 0)
        assert(op_trace.find("'path': 'group-item', 'count': 2") > 0)
        assert(op_trace.find(
            "'path': 'group-item/0/subgroup-double', 'value': 73.73") > 0)
        assert(op_trace.find(
            "'path': 'group-item/1/subgroup-double', 'value': 83.829") > 0)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

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

""" math_op.py Simple operation for testing task behavior."""

import math

import smtk
import smtk.attribute
import smtk.io
import smtk.operation

sbt_string = """
<SMTK_AttributeResource Version="7">
  <Definitions>
    <AttDef Type="MathOp" BaseType="operation">
      <ItemDefinitions>
        <Int Name="IntValue">
          <BriefDescription>Operation Will compute squared value</BriefDescription>
        </Int>
        <Double Name="DoubleValue" Label="Double Value">
          <BriefDescription>Operation Will compute square root</BriefDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="MathOpResult" BaseType="result">
      <ItemDefinitions>
        <Int Name="IntValueSquared" />
        <Double Name="DoubleValueRoot" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
    <View Type="Instanced" Title="MathOp" TopLevel="true"
      FilterByAdvanceLevel="false" FilterByCategory="false"
      UseScrollingContainer="false">
      <InstancedAttributes>
        <Att Type="MathOp" Name="MathOp" />
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
"""


class MathOp(smtk.operation.Operation):

    def __init__(self):
        smtk.operation.Operation.__init__(self)

    def name(self):
        return "Math Operation"

    def operateInternal(self):
        # Do some math
        int_value = self.parameters().findInt('IntValue').value()
        int_squared = int_value * int_value
        dbl_value = self.parameters().findDouble('DoubleValue').value()
        dbl_root = math.sqrt(math.fabs(dbl_value))

        # Add note to logger
        info = 'MathOp computed int_squared {}, dbl_root {}'.format(int_squared, dbl_root)
        self.log().addRecord(smtk.io.Logger.Info, info)

        # Return with success
        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)
        result.findInt('IntValueSquared').setValue(int_squared)
        result.findDouble('DoubleValueRoot').setValue(dbl_root)
        return result

    def createSpecification(self):
        spec = self.createBaseSpecification()

        reader = smtk.io.AttributeReader()
        hasErr = reader.readContents(spec, sbt_string, self.log())
        if hasErr:
            message = 'Error loading specification'
            self.log().addError(message)
            raise RuntimeError(message)
        return spec

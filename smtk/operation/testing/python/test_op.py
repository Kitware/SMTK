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

""" test_op.py:

Test the functionality of python operations.

"""
import smtk
import smtk.attribute
import smtk.io
import smtk.operation


class TestOp(smtk.operation.Operation):

    def __init__(self):
        smtk.operation.Operation.__init__(self)

    def name(self):
        return "test op"

    def operateInternal(self):
        smtk.WarningMessage(self.log(), 'My string is \"%s\"' %
                            self.parameters().findString('my string').value())

        # Return with success
        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)
        result.findString('my string').setValue(
            self.parameters().findString('my string').value())
        return result

    def createSpecification(self):
        spec = self.createBaseSpecification()

        opDef = spec.createDefinition('test op', 'operation')

        stringDef = smtk.attribute.StringItemDefinition.New('my string')
        stringDef.setDefaultValue('<unset>')
        opDef.addItemDefinition(stringDef)

        resultDef = spec.createDefinition('test result', 'result')

        stringDef = smtk.attribute.StringItemDefinition.New('my string')
        stringDef.setDefaultValue('<unset>')
        resultDef.addItemDefinition(stringDef)

        return spec

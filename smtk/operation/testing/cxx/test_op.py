#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

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

    def operateInternal(self):
        smtk.InfoMessage(logger, 'My string is %s' % stringDef.value())

        # Return with success
        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)
        return result

    def createSpecification(self):
        spec = smtk.attribute.Collection.create()

        opDef = spec.createDefinition('operation')

        debugDef = smtk.attribute.IntItemDefinition.New('debug level')
        debugDef.setDefaultValue(0)
        opDef.addItemDefinition(debugDef)

        stringDef = smtk.attribute.StringItemDefinition.New('my string')
        stringDef.setDefaultValue('<unset>')
        opDef.addItemDefinition(stringDef)

        resultDef = spec.createDefinition("result")

        outcomeDef = smtk.attribute.IntItemDefinition.New('outcome')
        resultDef.addItemDefinition(outcomeDef)

        logDef = smtk.attribute.StringItemDefinition.New('log')
        logDef.setIsOptional(True)
        logDef.setNumberOfRequiredValues(0)
        logDef.setIsExtensible(True)
        resultDef.addItemDefinition(logDef)

        createdDef = smtk.attribute.ComponentItemDefinition.New('created')
        createdDef.setNumberOfRequiredValues(0)
        createdDef.setIsExtensible(True)
        resultDef.addItemDefinition(createdDef)

        modifiedDef = smtk.attribute.ComponentItemDefinition.New('modified')
        modifiedDef.setNumberOfRequiredValues(0)
        modifiedDef.setIsExtensible(True)
        resultDef.addItemDefinition(modifiedDef)

        expungedDef = smtk.attribute.ComponentItemDefinition.New('expunged')
        expungedDef.setNumberOfRequiredValues(0)
        expungedDef.setIsExtensible(True)
        resultDef.addItemDefinition(expungedDef)

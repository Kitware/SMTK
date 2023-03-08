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

""" test_managers_operation.py:

Operation to test access to smtk managers and retrieval of smtk project in memory.
Works in conjunction with TestManagersAccess.cxx
"""

import smtk
import smtk.attribute
import smtk.common
import smtk.io
import smtk.operation
import smtk.project


class TestManagersOperation(smtk.operation.Operation):

    def __init__(self):
        smtk.operation.Operation.__init__(self)

    def name(self):
        return "test_managers_operation"

    def operateInternal(self):
        # Verify we can get the managers object, project manager, and current project
        managers = self.managers()
        if managers is None:
            self.log().addError('self.managers() returned None')
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

        proj_manager = managers.get('smtk::project::Manager')
        if proj_manager is None:
            self.log().addError('failed to get project manager from smtk::common::Managers')
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

        project_set = proj_manager.projectsSet()
        if len(project_set) != 1:
            self.log.addError(
                'project manager returned wrong number of projects' +
                ', should be 1 not {}'.format(len(project_set)))
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

        # Sanity check
        project = project_set.pop()
        if project.typeName() != 'xyzzy':
            self.log().addError(
                'wrong project typeName {}, should by xyzzy'.format(project.typeName()))
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)
        if project.name() != 'plugh':
            self.log().addError(
                'wrong project name {}, should by plugh'.format(project.name()))
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

        # All good
        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)
        return result

    def createSpecification(self):
        spec = self.createBaseSpecification()
        opDef = spec.createDefinition('test op', 'operation')
        resultDef = spec.createDefinition('test result', 'result')
        return spec

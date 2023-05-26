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

import smtk
import smtk.attribute
import smtk.project
import smtk.testing


class TestBasicProject(smtk.testing.TestCase):
    def setUp(self):
        # Initialize smtk managers
        self.res_manager = smtk.resource.Manager.create()
        self.op_manager = smtk.operation.Manager.create()

        smtk.attribute.Registrar.registerTo(self.res_manager)
        smtk.attribute.Registrar.registerTo(self.op_manager)

        smtk.operation.Registrar.registerTo(self.op_manager)
        self.op_manager.registerResourceManager(self.res_manager)

        smtk.project.Registrar.registerTo(self.op_manager)
        self.proj_manager = smtk.project.Manager.create(
            self.res_manager, self.op_manager)

        # Register 'basic' project type
        self.proj_manager.registerProject('basic', set(), set(), '0.0')

    def tearDown(self):
        self.res_manager = None
        self.op_manager = None
        self.proj_manager = None

    def test_add_resource(self):
        """"""
        project = self.proj_manager.createProject('basic')
        self.assertIsNotNone(project, 'failed to create project')

        # Create attribute resource
        att_resource = self.res_manager.createResource(
            'smtk::attribute::Resource')
        self.assertIsNotNone(att_resource)
        # Manage the resource
        self.res_manager.add(att_resource)
        self.assertEqual(len(self.res_manager.resources()), 1,
                         'Wrong number of project resources; should be 1 not {}'.format(project.resources().size()))

        # Add attribute resource to the project
        added = project.resources().add(att_resource, 'attributes')
        self.assertTrue(added, 'failed to add attribute resource to project')
        self.assertEqual(project.resources().size(), 1,
                         'Wrong number of project resources; should be 1 not {}'.format(project.resources().size()))

        resource_set = project.resources().findByRole('attributes')
        self.assertEqual(len(resource_set), 1,
                         'Wrong number of resources; should be 1 not {}'.format(project.resources().size()))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()

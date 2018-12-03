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

from __future__ import print_function
import os
import shutil
import sys
import unittest

import smtk
import smtk.attribute
import smtk.operation
import smtk.project
import smtk.session.mesh
import smtk.testing

PROJECT1 = 'project1'
SUCCEEDED = int(smtk.operation.Operation.Outcome.SUCCEEDED)


class TestProjectManager(unittest.TestCase):

    def setup(self):
        folder = os.path.join(smtk.testing.TEMP_DIR, PROJECT1)
        shutil.rmtree(folder)
        self.attribute_count = None
        self.pm = None    # project manager
        self.rm = None    # resource manager
        self.project = None

    def test_getProjectSpecification(self):
        pm = smtk.project.Manager.create()
        spec = pm.getProjectSpecification()
        self.assertEqual(spec.name(), 'new-project')

    def test_no_core_managers(self):
        pm = smtk.project.Manager.create()
        spec = pm.getProjectSpecification()
        outcome, project = pm.createProject(spec)
        self.assertNotEqual(outcome, SUCCEEDED)
        self.assertIsNone(project)

    def init_project_manager(self):
        # Initialize resource manager
        self.rm = smtk.resource.Manager.create()
        om = smtk.operation.Manager.create()
        smtk.session.mesh.Registrar.registerTo(self.rm)
        smtk.session.mesh.Registrar.registerTo(om)
        smtk.operation.Registrar.registerTo(om)
        om.registerResourceManager(self.rm)
        self.pm = smtk.project.Manager.create()
        self.pm.setCoreManagers(self.rm, om)

    def create_project(self, project_name):
        before_count = len(self.rm.resources())

        # Get specification
        spec = self.pm.getProjectSpecification()
        spec.findString('project-name').setValue(0, project_name)

        # Set project settings
        project_folder = os.path.join(smtk.testing.TEMP_DIR, project_name)
        spec.findDirectory('project-directory').setValue(0, project_folder)

        sim_template = os.path.join(
            smtk.testing.DATA_DIR, 'simulation-workflows', 'ACE3P', 'ACE3P.sbt')
        spec.findFile('simulation-template').setValue(0, sim_template)

        model_file = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'genesis', 'gun-1fourth.gen')
        spec.findFile('model-file').setValue(0, model_file)

        self.assertTrue(spec.isValid())

        # Create project
        outcome, project = self.pm.createProject(spec)
        self.assertEqual(outcome, SUCCEEDED)
        self.assertIsNotNone(project)
        self.project = project

        # Verify that 2 resources were created
        after_count = len(self.rm.resources())
        self.assertEqual(after_count - before_count, 2)

        # Verify that folders & files were created
        self.assertTrue(os.path.exists(project_folder))

        filenames = [
            '.cmbproject', 'default.sbi', 'gun-1fourth.gen', 'gun-1fourth.h5m', 'gun-1fourth.smtk']
        for f in filenames:
            path = os.path.join(project_folder, f)
            self.assertTrue(os.path.exists(path), '{}'.format(path))

        self.assertEqual(self.project.name(), project_name)
        self.assertEqual(self.project.directory(), project_folder)

    def modify_project(self):
        # Get simulation attributes
        att_resource = self.project.getResourceByRole(
            'smtk::attribute::Resource', 'default')
        before_count = len(att_resource.attributes())

        # Add 3 material attributes
        defn = att_resource.findDefinition('Material')
        att = att_resource.createAttribute(defn)
        att.findDouble('Epsilon').setValue(0.987)

        defn = att_resource.findDefinition('TEM3PElasticMaterial')
        att = att_resource.createAttribute(defn)
        type_item = att.findString('Material')
        type_item.setValue(0, 'Custom')
        density_item = type_item.findChild(
            'Density', smtk.attribute.SearchStyle.ACTIVE_CHILDREN).setValue(0, 1.95e11)

        defn = att_resource.findDefinition('TEM3PThermalMaterial')
        att = att_resource.createAttribute(defn)
        type_item = att.findString('NonlinearMaterial').setValue(0, 'AL6061')

        after_count = len(att_resource.attributes())
        self.assertEqual(after_count - before_count, 3)
        self.attribute_count = after_count

    def save_project(self):
        result = self.pm.saveProject()
        self.assertEqual(result, SUCCEEDED)

    def open_project(self):
        path = os.path.join(smtk.testing.TEMP_DIR, PROJECT1)
        outcome, project = self.pm.openProject(path)
        self.assertEqual(outcome, SUCCEEDED)

        model = project.getResourceByRole(
            'smtk::session::mesh::Resource', 'default')
        self.assertIsNotNone(model)

        att_resource = project.getResourceByRole(
            'smtk::attribute::Resource', 'default')
        self.assertIsNotNone(att_resource)

        att_count = len(att_resource.attributes())
        self.assertEqual(att_count, self.attribute_count)

    def close_project(self):
        before_count = len(self.rm.resources())
        result = self.pm.closeProject()
        self.assertEqual(result, SUCCEEDED)

        after_count = len(self.rm.resources())
        self.assertEqual(before_count - after_count, 2)

        # isLoaded, name, directory = self.pm.getStatus()
        # self.assertFalse(isLoaded)
        # self.assertEqual(name, '')
        # self.assertEqual(directory, '')

    def test_sequence(self):
        self.rm = None    # resource manager
        self.pm = None    # project manager
        self.attribute_count = 0

        try:
            self.init_project_manager()
            self.create_project(PROJECT1)
            self.modify_project()
            self.save_project()
            self.close_project()

            # Reopen project and check contents
            self.open_project()
        except:
            raise
        finally:
            # Remove project folder
            folder = os.path.join(smtk.testing.TEMP_DIR, PROJECT1)
            if os.path.exists(folder):
                shutil.rmtree(folder)

if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

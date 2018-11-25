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

    def test_getProjectSpecification(self):
        pm = smtk.project.Manager.create()
        spec = pm.getProjectSpecification()
        self.assertEqual(spec.name(), 'new-project')

    def init_project_manager(self):
        # Initialize resource manager
        rm = smtk.resource.Manager.create()
        om = smtk.operation.Manager.create()
        smtk.session.mesh.Registrar.registerTo(rm)
        smtk.session.mesh.Registrar.registerTo(om)
        smtk.operation.Registrar.registerTo(om)
        om.registerResourceManager(rm)
        self.pm = smtk.project.Manager.create()
        self.pm.setManagers(rm, om)

    def create_project(self, project_name):
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
        result = self.pm.createProject(spec)
        self.assertEqual(result, SUCCEEDED)

        # Verify that folders & files were created
        self.assertTrue(os.path.exists(project_folder))

        filenames = [
            '.cmbproject', 'default.sbi', 'gun-1fourth.gen', 'gun-1fourth.h5m', 'gun-1fourth.smtk']
        for f in filenames:
            path = os.path.join(project_folder, f)
            self.assertTrue(os.path.exists(path), '{}'.format(path))

        isLoaded, name, directory = self.pm.getStatus()
        self.assertTrue(isLoaded)
        self.assertEqual(name, project_name)
        self.assertEqual(directory, project_folder)

    def close_project(self):
        result = self.pm.closeProject()
        self.assertEqual(result, SUCCEEDED)

        isLoaded, name, directory = self.pm.getStatus()
        self.assertFalse(isLoaded)
        self.assertEqual(name, '')
        self.assertEqual(directory, '')

    def test_sequence(self):
        self.pm = None
        self.init_project_manager()
        self.create_project(PROJECT1)
        self.close_project()

        # Remove project folder
        # folder = os.path.join(smtk.testing.TEMP_DIR, PROJECT1)
        # shutil.rmtree(folder)

if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

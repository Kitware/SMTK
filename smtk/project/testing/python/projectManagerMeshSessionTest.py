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


class TestProjectManager(unittest.TestCase):

    def setup(self):
        folder = os.path.join(smtk.testing.TEMP_DIR, 'TestProjectManager')
        shutil.rmtree(folder)

    def test_getProjectSpecification(self):
        pm = smtk.project.Manager.create()
        spec = pm.getProjectSpecification()
        self.assertEqual(spec.name(), 'new-project')

    def test_createProject(self):
        # Initialize resource manager
        rm = smtk.resource.Manager.create()
        om = smtk.operation.Manager.create()
        smtk.session.mesh.Registrar.registerTo(rm)
        smtk.session.mesh.Registrar.registerTo(om)
        smtk.operation.Registrar.registerTo(om)
        om.registerResourceManager(rm)

        # Get specification
        pm = smtk.project.Manager.create()
        pm.setManagers(rm, om)
        spec = pm.getProjectSpecification()
        spec.findString('project-name').setValue(0, 'TestProjectManager')

        # Set project settings
        project_folder = os.path.join(
            smtk.testing.TEMP_DIR, 'TestProjectManager')
        spec.findDirectory('project-directory').setValue(0, project_folder)

        sim_template = os.path.join(
            smtk.testing.DATA_DIR, 'simulation-workflows', 'ACE3P', 'ACE3P.sbt')
        spec.findFile('simulation-template').setValue(0, sim_template)

        model_file = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'genesis', 'gun-1fourth.gen')
        spec.findFile('model-file').setValue(0, model_file)

        self.assertTrue(spec.isValid())

        # Create project
        result = pm.createProject(spec)
        succeeded = int(smtk.operation.Operation.Outcome.SUCCEEDED)
        self.assertEqual(result, succeeded)

        # Verify that folders & files were created
        self.assertTrue(os.path.exists(project_folder))

        filenames = [
            '.cmbproject', 'default.sbi', 'gun-1fourth.gen', 'gun-1fourth.h5m', 'gun-1fourth.smtk']
        for f in filenames:
            path = os.path.join(project_folder, f)
            self.assertTrue(os.path.exists(path), '{}'.format(path))

        isLoaded, name, directory = pm.getStatus()
        self.assertTrue(isLoaded)
        self.assertEqual(name, 'TestProjectManager')
        self.assertEqual(directory, project_folder)

if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

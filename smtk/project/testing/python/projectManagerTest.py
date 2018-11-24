from __future__ import print_function
import os
import shutil
import sys
import unittest

import smtk
import smtk.attribute
import smtk.operation
import smtk.project
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
        # Get specification
        pm = smtk.project.Manager.create()
        spec = pm.getProjectSpecification()
        spec.findString('project-name').setValue(0, 'TestProjectManager')

        # Set project settings
        folder = os.path.join(smtk.testing.TEMP_DIR, 'TestProjectManager')
        spec.findDirectory('project-directory').setValue(0, folder)

        sim_template = os.path.join(
            smtk.testing.DATA_DIR, 'simlation-workflows', 'ACE3P', 'ACE3P.sbt')
        spec.findFile('simulation-template').setValue(0, sim_template)

        model_file = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'genesis', 'gun-1fourth.gen')
        spec.findFile('model-file').setValue(0, model_file)

        self.assertTrue(spec.isValid())

        # Create project
        result = pm.createProject(spec)
        succeeded = int(smtk.operation.Operation.Outcome.SUCCEEDED)
        self.assertEqual(result, succeeded)

if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

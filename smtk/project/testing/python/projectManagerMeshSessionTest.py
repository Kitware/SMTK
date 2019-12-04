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

from __future__ import print_function
import os
import shutil
import sys
import unittest

import smtk
import smtk.attribute
import smtk.io
import smtk.mesh
import smtk.operation
import smtk.project
import smtk.session.mesh
import smtk.testing

PROJECT1 = 'project1'


class TestProjectManager(unittest.TestCase):

    def setup(self):
        folder = os.path.join(smtk.testing.TEMP_DIR, PROJECT1)
        shutil.rmtree(folder)
        self.attribute_count = None
        self.pm = None    # project manager
        self.rm = None    # resource manager
        self.om = None    # operation manager
        self.project = None
        self.NUMBER_OF_RESOURCES = 0  # for checking

    def test_getProjectSpecification(self):
        rm = smtk.resource.Manager.create()
        om = smtk.operation.Manager.create()
        pm = smtk.project.Manager.create(rm, om)
        spec = pm.getProjectSpecification()
        self.assertEqual(spec.name(), 'new-project')

    def test_invalid_project_specification(self):
        rm = smtk.resource.Manager.create()
        om = smtk.operation.Manager.create()
        pm = smtk.project.Manager.create(rm, om)
        spec = pm.getProjectSpecification()
        logger = smtk.io.Logger.instance()
        project = pm.createProject(spec, logger=logger)
        self.assertIsNone(project)

    def init_project_manager(self):
        # Initialize resource manager
        self.rm = smtk.resource.Manager.create()
        self.om = smtk.operation.Manager.create()

        smtk.attribute.Registrar.registerTo(self.rm)
        smtk.attribute.Registrar.registerTo(self.om)

        smtk.session.mesh.Registrar.registerTo(self.rm)
        smtk.session.mesh.Registrar.registerTo(self.om)

        smtk.operation.Registrar.registerTo(self.om)
        self.om.registerResourceManager(self.rm)

        self.pm = smtk.project.Manager.create(self.rm, self.om)

    def create_project(self, project_name):
        before_count = len(self.rm.resources())

        # Get specification
        spec = self.pm.getProjectSpecification()

        # Set project settings
        spec.findDirectory('workspace-path').setValue(0, smtk.testing.TEMP_DIR)
        spec.findString('project-folder').setValue(0, project_name)

        sim_template = os.path.join(
            smtk.testing.DATA_DIR, 'simulation-workflows', 'ACE3P', 'ACE3P.sbt')
        spec.findFile('simulation-template').setValue(0, sim_template)

        model_group_item = spec.findGroup('model-group')
        model_group_item.setIsEnabled(True)

        model_file = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'genesis', 'gun-1fourth.gen')
        model_group_item.find('model-file').setValue(0, model_file)

        # Make sure vtk-session option is off
        spec.findVoid('use-vtk-session').setIsEnabled(False)

        self.assertTrue(spec.isValid(), msg="project spec not valid")

        # Create project
        logger = smtk.io.Logger.instance()
        replace_existing_directory = True
        project = self.pm.createProject(
            spec, replace_existing_directory, logger)
        print('project: ', project)
        self.assertIsNotNone(project, msg=logger.convertToString())
        self.project = project
        self.NUMBER_OF_RESOURCES = 2

        # Verify that 2 resources were created
        self.assertEqual(
            len(self.project.resources()), self.NUMBER_OF_RESOURCES)
        after_count = len(self.rm.resources())
        self.assertEqual(after_count - before_count, self.NUMBER_OF_RESOURCES)

        # Verify that folders & files were created
        project_folder = os.path.join(smtk.testing.TEMP_DIR, project_name)
        self.assertTrue(os.path.exists(project_folder))

        filenames = [
            '.smtkproject', 'sbi.default.smtk',
            'gun-1fourth.gen', 'gun-1fourth.gen.h5m', 'gun-1fourth.gen.smtk']
        for f in filenames:
            path = os.path.join(project_folder, f)
            self.assertTrue(os.path.exists(path), '{}'.format(path))

        self.assertEqual(self.project.simulationCode(), 'ace3p')
        self.assertEqual(self.project.name(), project_name)
        self.assertEqual(self.project.directory(), project_folder)

    def modify_project(self):
        # Get simulation attributes
        att_resource = self.project.findAttributeResource('default')
        self.assertIsNotNone(att_resource)
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

        # Add second model
        before_count = len(self.rm.resources())
        second_model_file = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'exodus', 'disk_out_ref.ex2')
        copy_native_file = True
        use_vtk_session = False

        # Confirm that we cannot add 'default' model (because project already
        # has one)
        success = self.project.addModel(
            second_model_file, 'default', copy_native_file, use_vtk_session)
        self.assertFalse(success)

        # Confirm that we can add 'second' model
        success = self.project.addModel(
            second_model_file, 'second', copy_native_file, use_vtk_session)
        self.assertTrue(success)

        after_count = len(self.rm.resources())
        self.assertEqual(after_count - before_count, 1)
        self.NUMBER_OF_RESOURCES += 1

    def save_project(self):
        logger = smtk.io.Logger.instance()
        success = self.pm.saveProject(logger)
        self.assertTrue(success, msg=logger.convertToString())

        # Verify that second model files were created
        project_folder = os.path.join(
            smtk.testing.TEMP_DIR, self.project.name())
        filenames = [
            'disk_out_ref.ex2', 'disk_out_ref.ex2.h5m', 'disk_out_ref.ex2.smtk']
        for f in filenames:
            path = os.path.join(project_folder, f)
            self.assertTrue(os.path.exists(path), '{}'.format(path))

    def open_project(self):
        path = os.path.join(smtk.testing.TEMP_DIR, PROJECT1)
        # project = self.pm.openProject(path, smtk.io.Logger.instance())
        logger = smtk.io.Logger.instance()
        project = self.pm.openProject(path, logger)
        self.assertIsNotNone(project, msg=logger.convertToString())

        self.assertEqual(project.simulationCode(), 'ace3p')
        self.assertEqual(project.name(), PROJECT1)
        self.assertEqual(project.directory(), path)

        resources = project.resources()
        self.assertEqual(len(resources), self.NUMBER_OF_RESOURCES)

        for res in resources:
            if isinstance(res, smtk.attribute.Resource):
                att_count = len(res.attributes())
                self.assertEqual(att_count, self.attribute_count)
            else:
                self.assertTrue(isinstance(res, smtk.session.mesh.Resource))

        # Make sure we can find resources by identifer
        att_res = project.findAttributeResource('default')
        self.assertIsNotNone(att_res)
        model_res = project.findModelResource('default')
        self.assertIsNotNone(model_res)

        # And that we don't find things that aren't there
        nomesh_res = project.findMeshResource('default')
        self.assertIsNone(nomesh_res)
        noatt_res = project.findAttributeResource('second')
        self.assertIsNone(noatt_res)

    def close_project(self):
        before_count = len(self.rm.resources())
        logger = smtk.io.Logger.instance()
        success = self.pm.closeProject(logger)
        self.assertTrue(success, msg=logger.convertToString())

        after_count = len(self.rm.resources())
        self.assertEqual(before_count - after_count, self.NUMBER_OF_RESOURCES)

        project = self.pm.getCurrentProject()
        self.assertIsNone(project)

    def test_sequence(self):
        self.rm = None    # resource manager
        self.om = None    # operation manager
        self.pm = None    # project manager
        self.project = None
        self.attribute_count = 0
        self.NUMBER_OF_RESOURCES = 0

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

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
import sys
import unittest

import smtk
import smtk.attribute
import smtk.io
import smtk.operation
import smtk.resource
import smtk.testing

SIMATTS_FILENAME = 'simatts.sbi.smtk'
EXPATTS_FILENAME = 'expatts.sbi.smtk'
OPERATION_SUCCEEDED = 3


class TestReadReferenceItem(unittest.TestCase):

    def setUp(self):
        # Initialize resource & operation managers
        self.res_manager = smtk.resource.Manager.create()
        self.op_manager = smtk.operation.Manager.create()

        smtk.attribute.Registrar.registerTo(self.res_manager)
        smtk.attribute.Registrar.registerTo(self.op_manager)

        smtk.operation.Registrar.registerTo(self.op_manager)
        self.op_manager.registerResourceManager(self.res_manager)

        # Initialize resources
        self.sim_atts = None
        self.exp_atts = None

    def tearDown(self):
        self.res_manager = None
        self.op_manager = None
        self.sim_atts = None
        self.exp_atts = None

    def read_xml(self, path):
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        att_resource = smtk.attribute.Resource.create()
        err = reader.read(att_resource, path, True, logger)
        self.assertFalse(err, logger.convertToString())
        return att_resource

    def read_json(self, filename):
        input_path = os.path.join(smtk.testing.TEMP_DIR, filename)

        read_op = self.op_manager.createOperation(
            'smtk::operation::ReadResource')
        read_params = read_op.parameters()
        file_item = read_params.findFile('filename')
        file_item.setValue(input_path)

        result = read_op.operate()
        outcome = result.findInt('outcome')
        self.assertEqual(outcome.value(), OPERATION_SUCCEEDED)

        resource_item = result.findResource("resourcesCreated")
        resource = resource_item.value()
        self.assertIsNotNone(resource)
        return resource

    def write_json(self, resource, filename):
        output_path = os.path.join(smtk.testing.TEMP_DIR, filename)

        write_op = self.op_manager.createOperation(
            'smtk::operation::WriteResource')
        write_params = write_op.parameters()
        write_params.associate(resource)
        file_item = write_params.findFile('filename')
        file_item.setIsEnabled(True)
        file_item.setValue(output_path)

        result = write_op.operate()
        outcome = result.findInt('outcome')
        self.assertEqual(outcome.value(), OPERATION_SUCCEEDED)
        print('Wrote', output_path)

    def load_simatts(self, assign=True, write=True):
        """"""
        # Load simulation template
        sim_template = os.path.join(
            smtk.testing.DATA_DIR, 'attribute', 'attribute_collection', 'Basic2DFluid.sbt')
        self.sim_atts = self.read_xml(sim_template)
        self.res_manager.add(self.sim_atts)

        if assign:
            # Create material attribute
            defn = self.sim_atts.findDefinition('Material')
            self.assertIsNotNone(defn)
            att = self.sim_atts.createAttribute('a material', defn)
            self.assertIsNotNone(att)

        if write:
            self.write_json(self.sim_atts, SIMATTS_FILENAME)

    def init_expatts(self, assign=True, write=True):
        exp_template = """
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="exp_att">
      <ItemDefinitions>
        <Resource Name="attribute-resource">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" />
          </Accepts>
        </Resource>
        <String Name="note">
            <DefaultValue>xyzzy</DefaultValue>
        </String>
        <Component Name="attribute-component">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" filter="*" />
          </Accepts>
        </Component>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
"""
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        self.exp_atts = smtk.attribute.Resource.create()
        err = reader.readContents(self.exp_atts, exp_template, logger)
        self.assertFalse(err, logger.convertToString())
        self.res_manager.add(self.sim_atts)

        # Create attribute and assign reference items
        defn = self.exp_atts.findDefinition('exp_att')
        att = self.exp_atts.createAttribute('exp_att', defn)
        if assign:
            # Assign attribute-resource
            res_item = att.findResource('attribute-resource')
            ok = res_item.setValue(self.sim_atts)
            self.assertTrue(ok)

            # Assign attribute component
            comp_item = att.findComponent('attribute-component')
            mat = self.sim_atts.findAttribute('a material')
            self.assertIsNotNone(mat)
            ok = comp_item.setValue(mat)
            self.assertTrue(ok)

        self.res_manager.add(self.exp_atts)

        if write:
            self.write_json(self.exp_atts, EXPATTS_FILENAME)

    def check_expatts(self, exp_atts):
        att = exp_atts.findAttribute('exp_att')
        self.assertIsNotNone(att)

        res_item = att.findResource('attribute-resource')
        self.assertIsNotNone(res_item)
        self.assertEqual(res_item.numberOfValues(), 1)

        # The simatts resource should be loaded
        res = res_item.value()
        self.assertIsNotNone(res)

        # Check the component reference
        comp_item = att.findComponent('attribute-component')
        self.assertIsNotNone(comp_item)
        self.assertEqual(comp_item.numberOfValues(), 1)
        comp = comp_item.value()
        self.assertIsNotNone(comp)

    def remove_temp_files(self, filenames):
        debug_mode = False
        if debug_mode:
            print('NOT removing output files (debug_mode):', filenames)
            return

        # (else)
        for filename in filenames:
            path = os.path.join(smtk.testing.TEMP_DIR, filename)
            if os.path.exists(path):
                print('Deleting {}'.format(path))
                os.remove(path)

    def test_xmlIO(self):
        """Sanity check xml code"""
        expatts_xml_filename = 'expatts.sbi'
        try:
            self.load_simatts()
            self.init_expatts(write=False)
            self.check_expatts(self.exp_atts)

            writer = smtk.io.AttributeWriter()
            logger = smtk.io.Logger.instance()
            path = os.path.join(smtk.testing.TEMP_DIR, expatts_xml_filename)
            err = writer.write(self.exp_atts, path, logger)
            self.assertFalse(err)

            # Close both resources
            self.res_manager.remove(self.sim_atts)
            self.res_manager.remove(self.exp_atts)
            self.sim_atts = None
            self.exp_atts = None

            # Read back exp_atts file
            exp_atts = self.read_xml(path)
            self.res_manager.add(exp_atts)
            self.check_expatts(exp_atts)
        except:
            raise
        finally:
            self.remove_temp_files([SIMATTS_FILENAME, expatts_xml_filename])

    def test_jsonIO(self):
        """Test reference items with json I/O"""
        try:
            self.load_simatts()
            self.init_expatts()

            # Close both resources
            self.res_manager.remove(self.sim_atts)
            self.res_manager.remove(self.exp_atts)
            self.sim_atts = None
            self.exp_atts = None

            # Read back exp_atts file
            exp_atts = self.read_json(EXPATTS_FILENAME)
            self.res_manager.add(exp_atts)
            self.check_expatts(exp_atts)
        except:
            raise
        finally:
            self.remove_temp_files([SIMATTS_FILENAME, EXPATTS_FILENAME])


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

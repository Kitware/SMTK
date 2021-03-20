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
import sys
import unittest

import smtk
import smtk.attribute
import smtk.io
import smtk.testing


class TestReferenceItemChildren(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_reference_children(self):
        """"""
        # Load sbt file
        att_resource = smtk.attribute.Resource.New()
        sbt_path = os.path.join(smtk.testing.DATA_DIR, 'attribute',
                                'attribute_collection', 'refitem-categories.sbt')
        assert os.path.exists(sbt_path)
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        reader.read(att_resource, sbt_path, logger)

        # Enable Heat Transfer category
        att_resource.setActiveCategories(set(['Heat Transfer']))

        # Create solid material attribute
        real_defn = att_resource.findDefinition('material.solid')
        assert real_defn is not None
        real_att = att_resource.createAttribute(real_defn)
        assert real_att is not None

        # Create body attribute
        body_defn = att_resource.findDefinition('body')
        assert body_defn is not None
        body_att = att_resource.createAttribute(body_defn)
        assert body_att is not None

        # Assign material to body
        mat_item = body_att.findComponent('material')
        assert mat_item is not None
        assert mat_item.setValue(real_att)

        # Verify that body's material item has active child item "initialTemp"
        temp_item = mat_item.find(
            'initialTemp', smtk.attribute.SearchStyle.IMMEDIATE_ACTIVE)
        assert temp_item is not None

        # Verify that body's material item does NOT have active child "initialFlow"
        flow_item = mat_item.find(
            'initialFlow', smtk.attribute.SearchStyle.IMMEDIATE_ACTIVE)
        assert flow_item is None


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

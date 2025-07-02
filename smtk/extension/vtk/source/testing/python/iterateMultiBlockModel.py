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
import smtk.extension.vtk.source
import smtk.io
import smtk.markup
import smtk.testing

import vtk

import math

'''
  Construct a markup resource with analytic shapes as components and
  iterate the blocks of the output multiblock and access the model
  components using the blocks' metadata.
'''


class IterateMultiBlockModel(smtk.testing.TestCase):

    def setUp(self):

        smtk.loadPlugins()
        # smtk.mgrs = smtk.common.Managers.create()
        # self.rsrcMgr = smtk.resource.Manager.create()
        # self.operMgr = smtk.operation.Manager.create()
        # self.geomMgr = smtk.geometry.Manager.create()
        print(smtk.applicationContext())
        # Create a resource
        rsrc = smtk.markup.Resource.create()
        ss = smtk.markup.Component.CastTo(rsrc.createNodeOfType(
            smtk.string.Token('smtk::markup::Sphere')))
        bb = smtk.markup.Component.CastTo(
            rsrc.createNodeOfType(smtk.string.Token('smtk::markup::Box')))
        ss.setName('round thing')
        bb.setName('cornery')

        # Access the resulting resource and model
        self.resource = rsrc
        self.sphere = ss
        self.box = bb

    def testMultiBlockIteration(self):

        mbs = smtk.extension.vtk.source.vtkResourceMultiBlockSource()
        mbs.SetResource(self.resource)
        mbs.Update()

        components = mbs.GetOutput().GetBlock(0)
        geometries = []
        names = []

        component_names = set()

        it = components.NewIterator()
        it.InitTraversal()
        while not it.IsDoneWithTraversal():
            geometries.append(it.GetCurrentDataObject())
            component_names.add(mbs.GetComponent(
                it.GetCurrentMetaData()).name())
            it.GoToNextItem()
        geom = geometries[0]

        expected_component_names = {'round thing', 'cornery'}

        print('names are ', component_names)
        if len(component_names.symmetric_difference(expected_component_names)) > 0:
            raise RuntimeError(
                "retrieved component names do not match expected values")


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

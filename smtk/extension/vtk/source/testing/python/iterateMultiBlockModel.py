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
import smtk.mesh
import smtk.model
import smtk.session.mesh
import smtk.testing

import vtk

import math

'''
  Construct a uniform grid as a mesh model, use it to seed a model multiblock
  source, iterate the blocks of the output multiblock and access the model
  components using the blocks' metadata.
'''


class IterateMultiBlockModel(smtk.testing.TestCase):

    def setUp(self):

        # Construct a 'create uniform grid' operator
        op = smtk.session.mesh.CreateUniformGrid.create()

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError

        # Access the resulting resource and model
        self.resource = smtk.session.mesh.Resource.CastTo(
            res.find('resourcesCreated').value(0))
        modelEntity = res.find('created').value(0)
        self.model = smtk.model.Model(
            modelEntity.modelResource(), modelEntity.id())

    def testMultiBlockIteration(self):

        mbs = smtk.extension.vtk.source.vtkModelMultiBlockSource()
        mbs.SetModelResource(self.resource)
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

        expected_component_names = {
            'Domain', 'Lower X', 'Lower Y', 'Upper X', 'Upper Y'}

        if len(component_names.symmetric_difference(expected_component_names)) > 0:
            raise RuntimeError(
                "retrieved component names do not match expected values")


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

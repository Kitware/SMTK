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

import math

'''
  Import a triangulated terrain as a mesh model, add an auxiliary geometry as
  a glyph prototype, and create a bunch of glyph instances that are snapped to
  the surface of the terrain. Finally, check that all of the points (that were
  seeded above a certain z value) fall below their initial z value.
'''


class SnapPointsToSurface(smtk.testing.TestCase):

    def setUp(self):

        # Construct an import operator
        op = smtk.session.mesh.Import.create()

        # Set the import operators parameters
        fname = op.parameters().find('filename')
        fname.setValue(os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '3d', 'terrain.exo'))

        # We don't need to construct the BREP hierarchy for this model
        op.parameters().find('construct hierarchy').setIsEnabled(False)

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

        # Grab the top face (known to be named "Element Block 13")
        self.face = None
        for f in self.resource.findEntitiesOfType(int(smtk.model.FACE)):
            if f.name() == 'Element Block 13':
                self.face = f
                break

        if not self.face:
            raise Exception('Could not find top face')

        # Import auxiliary geometry describing the elevation for the mesh
        op = smtk.model.AddAuxiliaryGeometry.create()

        # Auxiliary geometry requires a model association
        op.parameters().associateEntity(self.model)

        # Set the location of the auxiliary data file
        fname = op.parameters().find('url')
        fname.setValue(os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'obj', 'cone.obj'))

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError

        # Access the resulting auxiliary geometry
        self.auxGeo = res.find("created").value()

    def testInstancePlacement(self):

        # Create an "Create Instances" operator
        op = smtk.model.CreateInstances.create()

        # Create several instances above the model and snap them to the top
        # surface
        op.parameters().associate(self.auxGeo)
        op.parameters().find('placement rule').setDiscreteIndex(1)
        op.parameters().find('volume of interest').item(0, 0).setValue(0, 57.)
        op.parameters().find('volume of interest').item(0, 0).setValue(1, 60.)
        op.parameters().find('volume of interest').item(1, 0).setValue(0, 2.)
        op.parameters().find('volume of interest').item(1, 0).setValue(1, 5.)
        op.parameters().find('volume of interest').item(2, 0).setValue(0, 0.1)
        op.parameters().find('volume of interest').item(2, 0).setValue(1, 0.2)
        op.parameters().find('sample size').setValue(10)
        op.parameters().find('snap to entity').setIsEnabled(True)
        op.parameters().find('snap to entity').setDiscreteIndex(1)
        op.parameters().find('entity').setValue(self.face.component())

        res = op.operate()

        if res.find('outcome').value() != int(smtk.operation.Operation.Outcome.SUCCEEDED):
            raise Exception('"Create Instances" operator failed')

        # Iterate over the coordinates of the instances to ensure that their
        # coordinates snapped to the model surface, which is below the box where
        # random points were seeded.
        for instance in self.resource.findEntitiesOfType(int(smtk.model.INSTANCE_ENTITY)):
            coords = instance.hasTessellation().coords()
            for counter, coord in enumerate(coords):
                if counter % 3 is 2 and coord >= 0.1:
                    raise Exception('coordinate %d did not snap' % counter / 3)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

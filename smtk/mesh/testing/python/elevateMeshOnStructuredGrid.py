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
import os
import sys
import unittest
import smtk
import smtk.extension.vtk.source
import smtk.mesh
import smtk.model
import smtk.session.mesh
import smtk.testing
import smtk.io.vtk

import math


class ValidatePoints(smtk.mesh.PointForEach):

    """
    Histogram the z-coordinates of the points in the mesh.
    """

    def __init__(self, nBins, minVal, maxVal):
        smtk.mesh.PointForEach.__init__(self)
        self.hist = [0] * nBins
        self.minVal = minVal
        self.maxVal = maxVal

    def forPoints(self, pointIds, xyz, doModify):
        counter = 0
        nPts = pointIds.size()
        for i in range(nPts):
            binNo = int((xyz[counter + 2] - self.minVal) /
                        (self.maxVal - self.minVal) * len(self.hist))
            if binNo < 0:
                binNo = 0
            elif binNo >= len(self.hist) - 1:
                binNo = len(self.hist) - 1
            self.hist[binNo] = self.hist[binNo] + 1
            counter += 3


class ElevateMeshOnStructuredGrid(smtk.testing.TestCase):

    def setUp(self):

        # Construct an import operator
        op = smtk.session.mesh.Import.create()

        # Set the import operators parameters
        fname = op.parameters().find('filename')
        fname.setValue(os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'atchafalaya_hydraulic_Mesh_18.h5m'))

        # We don't need to construct the BREP hierarchy for this model
        op.parameters().find('construct hierarchy').setIsEnabled(False)

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError

        # Access the resulting resource and model
        self.resource = smtk.session.mesh.Resource.CastTo(
            res.find('resource').value(0))
        modelEntity = res.find('created').value(0)
        self.model = smtk.model.Model(
            modelEntity.modelResource(), modelEntity.id())

        # Access the mesh set from the resource that comprises the model
        self.mesh = self.resource.resource().meshes()

        # Import auxiliary geometry describing the demographic data of the mesh
        op = smtk.model.AddAuxiliaryGeometry.create()

        # Auxiliary geometry requires a model association
        op.parameters().associateEntity(self.model)

        # Set the location of the auxiliary data file
        fname = op.parameters().find('url')
        fname.setValue(os.path.join(
            smtk.testing.DATA_DIR, 'dem', 'atchafalaya.dem'))

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError

        # Access the resulting auxiliary geometry
        self.auxGeo = res.find("created").value()

    def testMeshing2D(self):

        # Create an "elevate mesh" operator
        op = smtk.mesh.ElevateMesh.create()

        # Set the input data to look for auxiliary geometry
        op.parameters().find("input data").setToDefault()

        # Set the auxiliary geometry
        op.parameters().find("auxiliary geometry").setValue(
            smtk.model.AuxiliaryGeometry(self.auxGeo.modelResource(), self.auxGeo.id()).component())

        # Set a threshold range for the input data
        inputFilter = op.parameters().find("input filter")
        inputFilter.find("min threshold").setIsEnabled(True)
        inputFilter.find("min threshold").setValue(-5.)
        inputFilter.find("max threshold").setIsEnabled(True)
        inputFilter.find("max threshold").setValue(2.)

        # Set the interpolation scheme to be radial average
        op.parameters().find("interpolation scheme").setToDefault()

        # Set the radial average
        op.parameters().find("radius").setValue(100.)

        # For points that fall outside of our dataset, do not change their
        # z-coordinates
        op.parameters().find("external point values").setToDefault()

        # Set the mesh
        op.parameters().associate(smtk.mesh.Component.create(self.mesh))

        # Clamp the elevation values between -/+ 2
        outputFilter = op.parameters().find("output filter")
        outputFilter.find("min elevation").setIsEnabled(True)
        outputFilter.find("min elevation").setValue(-2.)
        outputFilter.find("max elevation").setIsEnabled(True)
        outputFilter.find("max elevation").setValue(2.)

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError

        # check the z bounds of the mesh to confirm that clamping was
        # successful
        extent = smtk.mesh.extent(self.mesh)
        if extent[4] < -2. or extent[5] > 2.:
            raise RuntimeError

        # Construct a histogram of the z-coordinates of the mesh points and
        # compare it to an expected set of values
        validatePoints = ValidatePoints(20, -2., 2.)
        smtk.mesh.for_each(self.mesh.points(), validatePoints)

        expected = [0, 0, 0, 0, 0,
                    5115, 2236, 1993, 2694, 2694,
                    4105, 1746, 523, 176, 57,
                    10, 4, 4, 1, 0]

        for i in range(20):
            if validatePoints.hist[i] != expected[i]:
                raise ValueError


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

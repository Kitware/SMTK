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
import smtk.bridge.mesh
import smtk.testing
import smtk.io.vtk
from smtk.simple import *

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

        # Construct a model manager
        self.mgr = smtk.model.Manager.create()

        # Access the mesh manager
        self.meshmgr = self.mgr.meshes()

        # Create a mesh session
        self.sess = self.mgr.createSession('mesh')
        SetActiveSession(self.sess)

        # Construct a model from a moab native mesh file
        op = self.sess.op('import')
        fname = op.find('filename')
        fname.setValue(os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'atchafalaya_hydraulic_Mesh_18.h5m'))

        # We don't need to construct the BREP hierarchy for this model
        op.find('construct hierarchy').setIsEnabled(False)

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
            raise ImportError

        # Access the resulting model
        self.model = res.find('created').value(0)

        # Access the mesh collections associated with the model
        associatedCollections = self.meshmgr.associatedCollections(self.model)

        # There should be only one, so let's just grab it from the list
        collection = associatedCollections[0]

        # Access the mesh set from the collection
        self.mesh = collection.meshes()

        # Import auxiliary geometry describing the demographic data of the mesh
        op = self.sess.op('add auxiliary geometry')

        # Auxiliary geometry requires a model association
        op.associateEntity(self.model)

        # Set the location of the auxiliary data file
        fname = op.find('url')
        fname.setValue(os.path.join(
            smtk.testing.DATA_DIR, 'dem', 'atchafalaya.dem'))

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
            raise ImportError

        # Access the resulting auxiliary geometry
        self.auxGeo = res.find("created").value()

    def testMeshing2D(self):

        # Create an "elevate mesh" operator
        op = self.sess.op('elevate mesh')

        # Set the input data to look for auxiliary geometry
        op.specification().find("input data").setToDefault()

        # Set the auxiliary geometry
        op.specification().find("auxiliary geometry").setValue(self.auxGeo)

        # Set a threshold range for the input data
        inputFilter = op.specification().find("input filter")
        inputFilter.find("min threshold").setValue(-5.)
        inputFilter.find("max threshold").setValue(2.)

        # Set the interpolation scheme to be radial average
        op.specification().find("interpolation scheme").setToDefault()

        # Set the radial average
        op.specification().find("radius").setValue(100.)

        # For points that fall outside of our dataset, do not change their
        # z-coordinates
        op.specification().find("external point values").setToDefault()

        # Set the mesh
        op.specification().find("mesh").appendValue(self.mesh)

        # Clamp the elevation values between -/+ 2
        outputFilter = op.specification().find("output filter")
        outputFilter.find("min elevation").setValue(-2.)
        outputFilter.find("max elevation").setValue(2.)

        # Execute the operator and check its results
        res = op.operate()
        if res.find('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
            raise RuntimeError

        # Construct a histogram of the z-coordinates of the mesh points and
        # compare it to an expected set of values
        validatePoints = ValidatePoints(20, -2., 2.)
        smtk.mesh.for_each(self.mesh.points(), validatePoints)

        expected = [8, 0, 0, 0, 0,
                    5115, 2236, 1992, 2694, 2693,
                    4103, 1739, 523, 177, 58,
                    9, 4, 6, 1, 0]

        for i in range(20):
            if validatePoints.hist[i] != expected[i]:
                raise ValueError


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

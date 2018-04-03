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
import smtk.mesh
import smtk.model
import smtk.operation
import smtk.bridge.mesh
import smtk.testing
from smtk.simple import *


class LoadExodusFile(smtk.testing.TestCase):

    def setUp(self):

        # Construct a resource manager
        self.resourceManager = smtk.resource.Manager.create()

        # Register mesh resource to the resource manager (this allows the
        # resource manager to track mesh resources)
        smtk.bridge.mesh.registerResources(self.resourceManager)

        # Construct an operation manager
        self.operationManager = smtk.operation.Manager.create()

        # Register operation and mesh operations to the operation manager
        # to provide us with resource I/O.
        smtk.operation.registerOperations(self.operationManager)
        smtk.bridge.mesh.registerOperations(self.operationManager)

        # Register resource manager to the operation manager
        self.operationManager.registerResourceManager(self.resourceManager)

    def testLoadExodusFile(self):
        # Set up the path to the test's input file
        modelFile = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'exodus', 'disk_out_ref.ex2')

        # Load the input file
        loadOp = self.operationManager.createOperation(
            'smtk::operation::ImportResource')
        loadOp.parameters().find('filename').setValue(modelFile)
        loadRes = loadOp.operate()

        if loadRes.findInt('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise ImportError

        # Access the resource
        resource = smtk.model.Manager.CastTo(loadRes.find('resource').value())


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

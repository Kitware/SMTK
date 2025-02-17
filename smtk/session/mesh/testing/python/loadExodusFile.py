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
import smtk.mesh
import smtk.model
import smtk.operation
import smtk.session.mesh
import smtk.testing


class LoadExodusFile(smtk.testing.TestCase):

    def setUp(self):

        # Construct a resource manager
        self.resourceManager = smtk.resource.Manager.create()

        # Construct an operation manager
        self.operationManager = smtk.operation.Manager.create()

        # Register mesh resource to the resource manager (this allows the
        # resource manager to track mesh resources)
        smtk.session.mesh.Registrar.registerTo(self.resourceManager)

        # Register mesh operations to the operation manager (this lets the
        # operation manager call the mesh session's load methods)
        smtk.session.mesh.Registrar.registerTo(self.operationManager)

        # Register operation operations to the operation manager (this lets the
        # operation manager create a generic "ImportResource" operation that can
        # load any resource it knows about).
        smtk.operation.Registrar.registerTo(self.operationManager)

        # Register resource manager to the operation manager
        self.operationManager.registerResourceManager(self.resourceManager)

    def testLoadExodusFile(self):
        # Set up the path to the test's input file
        modelFile = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'exodus', 'disk_out_ref.ex2')

        # Create an ImportResource operation (alternatively, we could have just
        # created a smtk.session.mesh.Import operation directly, avoiding the
        # need for all of the setup registration. This is a test, though, so
        loadOp = self.operationManager.createOperation(
            'smtk::operation::ImportResource')
        loadOp.parameters().find('filename').setValue(modelFile)
        loadRes = loadOp.operate()

        if loadRes.findInt('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError

        # Access the resource
        resource = smtk.model.Resource.CastTo(
            loadRes.find('resourcesCreated').value())


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

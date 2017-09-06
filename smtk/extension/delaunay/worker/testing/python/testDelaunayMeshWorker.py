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

""" testDelaunayMeshWorker.py:

Test/demonstrate the ability to use the Delaunahy mesh worker from within the
Python environment. In this test, we provide the worker factory with search
paths to the install and build locations of the mesh worker, facilitating the
Remus' execution of the worker (i.e. no process spawining is required by the
user).

"""

import os
import sys
import unittest
import smtk
import smtk.attribute
import smtk.model
import smtk.model.remus
import smtk.bridge.polygon
import smtk.testing
from smtk.simple import *


class TestDelaunayMeshWorker(smtk.testing.TestCase):

    def setUp(self):
        # create model manager
        self.mgr = smtk.model.Manager.create()

        # create a polygon session and set it as the active session
        self.sess = self.mgr.createSession('polygon')
        SetActiveSession(self.sess)

        # locate our input file from the testing data directory
        self.modelFile = os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'boxWithHole.smtk')

        # import the file and access the resulting model
        self.model = LoadSMTKModel(self.modelFile)[0]

        # start an instance of our mesh server
        self.meshServerLauncher = smtk.model.remus.MeshServerLauncher()

    def testMeshing2DDelaunay(self):
        # if we cannot load the delaunay and delaunay.worker modules, then the
        # test fails.
        try:
            import smtk.mesh.delaunay
            import smtk.mesh.delaunay.worker
        except:
            print("Could not load Delaunay mesh worker:", sys.exc_info()[0])
            raise

        # provide the worker factory with a list of locations where it can
        # find the job requirements file (*.rw) and associated executable.
        paths = smtk.common.Paths()
        for search_path in paths.workerSearchPaths():
            self.meshServerLauncher.addWorkerSearchDirectory(search_path)

        # access the meshing attribute collection for the delaunay worker
        meshingAttributes = smtk.mesh.delaunay.worker.meshing_attributes()

        # the mesher may require an attribute of type "Globals" that is named
        # "Globals". When run through ModelBuilder, this attribute is created
        # automatically as an instanced attribute in a view. Since we do not
        # have any view to the operator attributes, we must instantiate this
        # attribute ourselves.
        #
        # TODO: the construction of default attributes should be an automated
        #       process that is callable within the attribute collection.
        meshingAttributes.createAttribute("Globals", "Globals")

        # now that we have a minimally sufficient attribute collection, we must
        # stringify it so it can be passed to the mesh operator.
        logger = smtk.io.Logger()
        writer = smtk.io.AttributeWriter()
        meshingAttributesStr = writer.writeContents(meshingAttributes, logger)

        # create a mesh operator
        mesher = self.sess.op('mesh')

        # set the model to be meshed
        mesher.specification().findModelEntity("model").setValue(self.model)

        # set the client endpoint expected by our mesh server
        mesher.specification().findString("endpoint").setValue(
            self.meshServerLauncher.clientEndpoint())

        # set the job requirements for this mesh operation
        mesher.specification().findString("remusRequirements").setValue(
            smtk.mesh.delaunay.worker.job_requirements())

        # set the stringified mesh specification that describes the mesher's
        # input parameters
        mesher.specification().findString(
            "meshingControlAttributes").setValue(meshingAttributesStr)

        # execute the mesh operator
        result = mesher.operate()

        # confirm that the operator successfully executed
        self.assertEqual(
            result.findInt('outcome').value(0),
           smtk.model.OPERATION_SUCCEEDED)

        # access the face that was meshed
        face = self.mgr.findEntitiesOfType(int(smtk.model.FACE))[0]

        # access the mesh collection associated with the model
        triangulatedFace = self.mgr.meshes().associatedCollections(face)[0]

        # confirm that the number of points and cells match our expectations
        self.assertEqual(triangulatedFace.points().size(), 12)
        self.assertEqual(triangulatedFace.cells().size(), 10)

        # if VTK is available, visualize the mesh
        if self.interactive() and self.haveVTK() and self.haveVTKExtension():
            self.startRenderTest()
            self.addMeshToScene(triangulatedFace.meshes())
            cam = self.renderer.GetActiveCamera()
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.interact()

    def tearDown(self):
        # shut down our mesh server
        self.meshServerLauncher.terminate()

if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()

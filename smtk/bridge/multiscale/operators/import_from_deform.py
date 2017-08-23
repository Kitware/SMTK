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

""" import_from_deform.py:

For the AFRL Materials Phase I Demo, this operator takes the DEFORM point
tracking file, DEFORM element file, the attribute to use when generating the
zones, statistics to use to generate representative volume elements (RVEs)
for each of the zones, and the path to the Dream3D executable
"PipelineRunner". With these data, Using Dream3D, the DEFORM model is
imported, cells are then clustered into zones according to the input
attribute, and a microscale profile is generated for each zone. The model is
partitioned into a number of zones equal to the number of microscale
statistics feature parameters provided by the user.

"""
import AFRLDir

import sys

afrlDir = AFRLDir.description.replace('\n', '') + '/CMBPreprocessingScripts'

if afrlDir not in sys.path:
    sys.path.append(afrlDir)

import Dream3DPipeline

import os
import smtk
import smtk.io.vtk
import smtk.mesh
import smtk.model
import smtk.bridge.mesh
import smtk.bridge.multiscale
import subprocess
import vtk


@smtk.model.operator("import from deform", smtk.bridge.multiscale.Session)
class import_from_deform(smtk.model.Operator):

    def __init__(self):
        smtk.model.Operator.__init__(self)

    def name(self):
        return "import from deform"

    def className(self):
        return self.__name__

    def operateInternal(self):

        # Access the DEFORM point and element files
        point_file = self.specification().findFile('point-file').value(0)
        element_file = self.specification().findFile('element-file').value(0)

        # Access the timestep to process
        timestep = self.specification().findInt('timestep').value(0)

        # Access the Dream3D PipelineRunner exectuable
        pipeline_executable = self.specification().findFile(
            'pipeline-executable').value(0)

        # Access the name of the attribute to use for zoning
        attribute = self.specification().findString('attribute').value(0)

        # Access the microscale statistics parameters
        stats = self.specification().findGroup('stats')

        # Access the Dream3D ouptut file
        output_file = self.specification().findFile('output-file').value(0)

        # The location of the template pipeline is hard-coded w.r.t. the AFRL
        # directory
        template_pipeline_file = AFRLDir.description.replace(
            '\n', '') + '/Dream3DPipelines/Pipelines/DREAM3D_Phase1_Pipeline.json'

        # Extract the parameters into python lists
        mu = []
        sigma = []
        min_cutoff = []
        max_cutoff = []

        for i in range(stats.numberOfGroups()):
            for p in ['mu', 'sigma', 'min_cutoff', 'max_cutoff']:
                eval(p).append(
                    smtk.attribute.to_concrete(stats.find(i, p)).value(0))

        # Access the Dream3D output file
        output_file = self.specification().findFile('output-file').value(0)

        # Ensure that the executable is, in fact, an executable
        pipeline = Dream3DPipeline.which(pipeline_executable)
        if pipeline is None:
            print('Cannot find PipelineRunner')
            return self.createResult(smtk.model.OPERATION_FAILED)

        # Generate the Dream3D pipeline for this operation
        pipeline_file = \
            Dream3DPipeline.generate_pipeline(template_pipeline_file,
                                              point_file, timestep,
                                              element_file,
                                              attribute, mu, sigma,
                                              min_cutoff, max_cutoff,
                                              output_file)
        # Execute the Dream3D pipeline
        pipelineargs = [pipeline, '-p', '%s' % os.path.abspath(pipeline_file)]
        subprocess.call(pipelineargs)

        # Remove the pipeline file
        os.remove(pipeline_file)

        # Read DREAM3D Xdmf file as a VTK data object
        xdmfReader = vtk.vtkXdmfReader()
        xdmfReader.SetFileName(os.path.splitext(output_file)[0] + '.xdmf')
        xdmfReader.Update()

        dataNames = [xdmfReader.GetOutputDataObject(0).GetMetaData(i)
                     .Get(vtk.vtkCompositeDataSet.NAME()) for i in
                     xrange(xdmfReader.GetNumberOfGrids())]

        volumeDataContainer = xdmfReader.GetOutputDataObject(0).GetBlock(
            dataNames.index("VolumeDataContainer"))

        # Import the vtk data object as an SMTK mesh
        cnvrt = smtk.io.vtk.ImportVTKData()
        collection = cnvrt(
            volumeDataContainer, self.meshManager(), 'ZoneIds')

        # Ensure that the import succeeded
        if not collection or not collection.isValid():
            return self.createResult(smtk.model.OPERATION_FAILED)

        # Assign its model manager to the one associated with this session
        collection.modelManager = self.manager()
        collection.name("DEFORM mesh")

        # Construct the topology
        self.activeSession().addTopology(smtk.bridge.mesh.Topology(collection))

        # Our collections will already have a UUID, so here we create a model
        # given the model manager and UUID
        model = self.manager().insertModel(
            collection.entity(), 2, 2, "DEFORM model")
        self.session().declareDanglingEntity(model)

        # Set the model's session to point to the current session
        model.setSession(
            smtk.model.SessionRef(self.manager(), self.session().sessionId()))

        collection.associateToModel(model.entity())

        # If we don't call "transcribe" ourselves, it never gets called.
        self.activeSession().transcribe(
            model, smtk.model.SESSION_EVERYTHING, False)

        result = self.createResult(smtk.model.OPERATION_SUCCEEDED)

        resultModels = result.findModelEntity("model")
        resultModels.setValue(model)

        created = result.findModelEntity("created")
        created.setNumberOfValues(1)
        created.setValue(model)
        created.setIsEnabled(True)

        result.findModelEntity("mesh_created").setValue(model)

        # Return with success
        return result

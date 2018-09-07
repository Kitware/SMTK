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

import import_from_deform_xml

import os
import os.path
import smtk
import smtk.attribute
import smtk.session.mesh
import smtk.session.multiscale
import smtk.io
import smtk.io.vtk
import smtk.mesh
import smtk.model
import smtk.operation
import subprocess
import vtk


class import_from_deform(smtk.operation.Operation):

    def __init__(self):
        smtk.operation.Operation.__init__(self)

    def name(self):
        return "import from deform"

    def operateInternal(self):
        # Access the DEFORM point and element files
        point_file = self.parameters().find('point-file').value(0)
        element_file = self.parameters().find('element-file').value(0)

        # Access the timestep to process
        timestep = self.parameters().find('timestep').value(0)

        # Access the Dream3D PipelineRunner exectuable
        pipeline_executable = self.parameters().find(
            'pipeline-executable').value(0)

        # Access the name of the attribute to use for zoning
        attribute = self.parameters().find('attribute').value(0)

        # Access the microscale statistics parameters
        stats = self.parameters().find('stats')

        # Access the Dream3D ouptut file
        output_file = self.parameters().find('output-file').value(0)

        # Create a resource and session
        resource = smtk.session.multiscale.Resource.create()
        session = smtk.session.multiscale.Session.create()
        resource.setLocation(point_file + '.smtk')
        resource.setSession(session)

        # The location of the template pipeline is hard-coded w.r.t. the AFRL
        # directory
        template_pipeline_file = AFRLDir.description.replace('\n', '') + \
            '/Dream3DPipelines/Pipelines/DREAM3D_Phase1_Pipeline.json'

        # Extract the parameters into python lists
        mu = []
        sigma = []
        min_cutoff = []
        max_cutoff = []

        for i in range(stats.numberOfGroups()):
            for p in ['mu', 'sigma', 'min_cutoff', 'max_cutoff']:
                eval(p).append(stats.find(i, p).value(0))

        # Access the Dream3D output file
        output_file = self.parameters().find('output-file').value(0)

        # Ensure that the executable is, in fact, an executable
        pipeline = Dream3DPipeline.which(pipeline_executable)
        if pipeline is None:
            print('Cannot find PipelineRunner at \'',
                  pipeline_executable, '\'')
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

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
        f = open('/Users/tjcorona/Desktop/out.txt', 'w')
        subprocess.call(pipelineargs, shell=True)

        # Remove the pipeline file
#        os.remove(pipeline_file)

        # Check for the resulting xdmf file
        if not os.path.isfile(output_file):
            smtk.ErrorMessage(
                smtk.io.Logger.instance(), "No DREAM3D pipeline output")
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

        # Read DREAM3D Xdmf file as a VTK data object
        xdmfReader = vtk.vtkXdmfReader()
        xdmfReader.SetFileName(os.path.splitext(output_file)[0] + '.xdmf')
        xdmfReader.Update()

        dataNames = [xdmfReader.GetOutputDataObject(0).GetMetaData(i)
                     .Get(vtk.vtkCompositeDataSet.NAME()) for i in
                     range(xdmfReader.GetNumberOfGrids())]

        volumeDataContainer = xdmfReader.GetOutputDataObject(0).GetBlock(
            dataNames.index("VolumeDataContainer"))

        # Import the vtk data object as an SMTK mesh
        cnvrt = smtk.io.vtk.ImportVTKData()
        collection = cnvrt(volumeDataContainer, resource.meshes(), 'ZoneIds')

        # Ensure that the import succeeded
        if not collection or not collection.isValid():
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

        # Assign its model manager to the one associated with this session
        collection.modelResource = resource
        collection.name("DEFORM mesh")

        # Construct the topology
        session.addTopology(smtk.session.mesh.Topology(collection))

        # Our collections will already have a UUID, so here we create a model
        # given the model manager and UUID
        model = resource.insertModel(collection.entity(), 2, 2, "DEFORM model")

        # Declare the model as "dangling" so it will be transcribed
        session.declareDanglingEntity(model)

        # Set the model's session to point to the current session
        model.setSession(smtk.model.SessionRef(resource, session.sessionId()))

        collection.associateToModel(model.entity())

        # If we don't call "transcribe" ourselves, it never gets called.
        session.transcribe(model, smtk.model.SESSION_EVERYTHING, False)

        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)

        created = result.findResource("resource")
        created.setValue(resource)

        resultModels = result.findComponent("model")
        resultModels.setValue(model.component())

        created = result.findComponent("created")
        created.setNumberOfValues(1)
        created.setValue(model.component())
        created.setIsEnabled(True)

        result.findComponent("mesh_created").setValue(model.component())

        # Return with success
        return result

    def createSpecification(self):
        spec = smtk.attribute.Resource.create()
        reader = smtk.io.AttributeReader()
        reader.readContents(
            spec, import_from_deform_xml.description, self.log())
        return spec

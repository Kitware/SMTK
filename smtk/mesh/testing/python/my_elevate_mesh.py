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

""" my_elevate_mesh.py:

An example of a runtime-loadable python operator.

"""
import smtk
import smtk.mesh
import smtk.model

import math


@smtk.model.operator("my elevate mesh", smtk.model.Session)
class MyElevateMesh(smtk.model.Operation):

    def __init__(self):
        smtk.model.Operation.__init__(self)

    def name(self):
        return "my elevate mesh"

    def className(self):
        return self.__name__

    @staticmethod
    def description():
        descr = '''
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeSystem Version="2">
<Definitions>
  <AttDef Type="my elevate mesh" Label="Mesh - My Elevate" BaseType="operator">
    <BriefDescription>Demonstrate how to write a python operator.</BriefDescription>
    <ItemDefinitions>
      <MeshEntity Name="mesh" NumberOfRequiredValues="1"/>
    </ItemDefinitions>
  </AttDef>
  <AttDef Type="result(my elevate mesh)" BaseType="result">
    <ItemDefinitions>
      <MeshEntity Name="mesh_modified" NumberOfRequiredValues="1" AdvanceLevel="11"/>
      <ModelEntity Name="tess_changed" NumberOfRequiredValues="0" Extensible="1" AdvanceLevel="11"/>
    </ItemDefinitions>
  </AttDef>
</Definitions>
</SMTK_AttributeSystem>
            '''
        return descr

    def operateInternal(self):
        # Access the mesh and filename from the specification
        mesh = self.specification().findMesh('mesh').value(0)

        # Compute the extent of the mesh
        extent = smtk.mesh.extent(mesh)

        # Construct a function that takes in an array of three coordinates and
        # returns an array of three coordinates.
        def sinusoid(xyz):
            return [xyz[0], xyz[1],
                    math.sin(2. * math.pi * (xyz[0] - extent[0]) / (extent[1] - extent[0])) *
                    math.sin(2. * math.pi * (xyz[1] - extent[2]) / (extent[3] - extent[2]))]

        # Warp the mesh nodes according to the above function. Each node
        # coordinate triplet is filtered through the function, and its values
        # are replaced by the function's output.
        smtk.mesh.applyWarp(sinusoid, mesh)

        result = self.createResult(smtk.model.OPERATION_SUCCEEDED)

        # We have modified the mesh by warping the z coordinates of its nodes.
        # We therefore must mark the mesh as having been modified.
        modified_meshes = result.findMesh("mesh_modified")
        modified_meshes.setValue(mesh)

        # The mesh we modified may be used as a model entity's tessellation. If
        # so, we must mark the entity as having a modified tessellation.
        modified_entities = result.findModelEntity("modified")
        changed_tess = result.findModelEntity("tess_changed")

        entities = mesh.modelEntities()
        if len(entities) != 0:
            model = entities[0].owningModel()
            modified_entities.appendValue(model)
            changed_tess.appendValue(model)

        return result

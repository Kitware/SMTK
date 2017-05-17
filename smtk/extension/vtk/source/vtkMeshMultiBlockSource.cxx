//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h" // for Array name

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Volume.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Manager.h"

#include "smtk/extension/vtk/io/ExportVTKData.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"

#include <errno.h>
#include <inttypes.h>
#include <iostream>
#include <stdlib.h>

using namespace smtk::model;

vtkStandardNewMacro(vtkMeshMultiBlockSource);
vtkCxxSetObjectMacro(vtkMeshMultiBlockSource, CachedOutput, vtkMultiBlockDataSet);

vtkMeshMultiBlockSource::vtkMeshMultiBlockSource()
{
  this->SetNumberOfInputPorts(0);
  this->CachedOutput = NULL;
  this->ModelEntityID = NULL;
  this->MeshCollectionID = NULL;
  this->AllowNormalGeneration = 0;
}

vtkMeshMultiBlockSource::~vtkMeshMultiBlockSource()
{
  this->SetCachedOutput(NULL);
  this->SetModelEntityID(NULL);
  this->SetMeshCollectionID(NULL);
}

void vtkMeshMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: " << this->m_modelMgr.get() << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
  os << indent << "ModelEntityID: " << this->ModelEntityID << "\n";
  os << indent << "MeshCollectionID: " << this->MeshCollectionID << "\n";
  os << indent << "AllowNormalGeneration: " << (this->AllowNormalGeneration ? "ON" : "OFF") << "\n";
}

/// Set the SMTK model to be displayed.
void vtkMeshMultiBlockSource::SetModelManager(smtk::model::ManagerPtr model)
{
  if (this->m_modelMgr == model)
  {
    return;
  }
  this->m_modelMgr = model;
  this->SetMeshManager(model ? model->meshes() : smtk::mesh::ManagerPtr());
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::ManagerPtr vtkMeshMultiBlockSource::GetModelManager()
{
  return this->m_modelMgr;
}

/// Set the SMTK mesh manager
void vtkMeshMultiBlockSource::SetMeshManager(smtk::mesh::ManagerPtr meshmgr)
{
  if (this->m_meshMgr == meshmgr)
  {
    return;
  }
  this->m_meshMgr = meshmgr;
  this->Modified();
}

/// Get the SMTK mesh manager.
smtk::mesh::ManagerPtr vtkMeshMultiBlockSource::GetMeshManager()
{
  return this->m_meshMgr;
}

/// Get the map from model entity UUID to the block index in multiblock output
void vtkMeshMultiBlockSource::GetMeshSet2BlockIdMap(
  std::map<smtk::mesh::MeshSet, unsigned int>& uuid2mid)
{
  uuid2mid.clear();
  uuid2mid.insert(this->m_Meshset2BlockIdMap.begin(), this->m_Meshset2BlockIdMap.end());
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkMeshMultiBlockSource::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(NULL);
}

/// Add customized block info.
/// Mapping from UUID to block id
static void internal_AddBlockEntityInfo(const smtk::mesh::MeshSet& mesh,
  const smtk::model::EntityRef& entityref, const vtkIdType& blockId, vtkPolyData* poly,
  std::map<smtk::mesh::MeshSet, unsigned int>& mesh2BlockId)
{
  mesh2BlockId[mesh] = static_cast<unsigned int>(blockId);

  // Add Entity UUID to fieldData
  vtkNew<vtkStringArray> uuidArray;
  uuidArray->SetNumberOfComponents(1);
  uuidArray->SetNumberOfTuples(1);
  uuidArray->SetName(vtkModelMultiBlockSource::GetEntityTagName());
  uuidArray->SetValue(0, entityref.entity().toString());
  poly->GetFieldData()->AddArray(uuidArray.GetPointer());
}

/// Add customized block info.
/// Mapping from MeshSet to block id
/// 'Volume' field array to color by volume
static void internal_AddBlockInfo(const smtk::mesh::CollectionPtr& meshcollect,
  const smtk::model::EntityRef& entityref, const smtk::mesh::MeshSet& blockmesh,
  const smtk::model::EntityRef& bordantCell, const vtkIdType& blockId, vtkPolyData* poly,
  std::map<smtk::mesh::MeshSet, unsigned int>& mesh2BlockId)
{
  meshcollect->setIntegerProperty(blockmesh, "block_index", blockId);

  internal_AddBlockEntityInfo(blockmesh, entityref, blockId, poly, mesh2BlockId);

  smtk::model::EntityRefs vols;
  if (bordantCell.isValid() && bordantCell.isVolume())
    vols.insert(bordantCell);
  if (vols.size())
  {
    // Add volume UUID to fieldData
    vtkNew<vtkStringArray> volArray;
    volArray->SetNumberOfComponents(1);
    volArray->SetNumberOfTuples(vols.size());
    int ai = 0;
    for (smtk::model::EntityRefs::iterator it = vols.begin(); it != vols.end(); ++it, ++ai)
    {
      volArray->SetValue(ai, (*it).entity().toString());
    }
    volArray->SetName(vtkModelMultiBlockSource::GetVolumeTagName());
    poly->GetFieldData()->AddArray(volArray.GetPointer());
  }
}

/// Loop over the model generating blocks of polydata.
void vtkMeshMultiBlockSource::GenerateNormals(
  vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals)
{
  if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
  {
    bool reallyNeedNormals = genNormals;
    if (entityref.isValid() && entityref.hasIntegerProperty("generate normals"))
    { // Allow per-entity setting to override per-model setting
      const IntegerList& prop(entityref.integerProperty("generate normals"));
      reallyNeedNormals = (!prop.empty() && prop[0]);
    }
    if (reallyNeedNormals)
    {
      this->m_normalGenerator->SetInputDataObject(pd);
      this->m_normalGenerator->Update();
      pd->ShallowCopy(this->m_normalGenerator->GetOutput());
    }
  }
}

/// Generate single block of polydata for single meshset
void vtkMeshMultiBlockSource::GenerateRepresentationForSingleMesh(const smtk::mesh::MeshSet& meshes,
  vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals)
{
  if (!meshes.is_empty())
  {
    //we want all 0d, 1d, 2d, and shells of 3d elments
    smtk::mesh::MeshSet shell = meshes.subset(smtk::mesh::Dims3).extractShell();
    smtk::mesh::MeshSet twoD = meshes.subset(smtk::mesh::Dims2);
    smtk::mesh::MeshSet oneD = meshes.subset(smtk::mesh::Dims1);
    smtk::mesh::MeshSet zeroD = meshes.subset(smtk::mesh::Dims0);

    smtk::mesh::MeshSet toRender = shell;
    toRender.append(twoD);
    toRender.append(oneD);
    toRender.append(zeroD);

    smtk::extension::vtk::io::ExportVTKData exportVTKData;
    exportVTKData(toRender, pd);

    this->GenerateNormals(pd, entityref, genNormals);

    // Point-coordinates attribute
    vtkNew<vtkDoubleArray> pointCoords;
    pointCoords->ShallowCopy(pd->GetPoints()->GetData());
    pointCoords->SetName("PointCoordinates");
    pd->GetPointData()->AddArray(pointCoords.GetPointer());
  }
}

/// Recursively find all the entities with meshes
/// \a entityrefMap is map from entityref to its parent cell entity and its meshset
void vtkMeshMultiBlockSource::FindEntitiesWithMesh(const smtk::mesh::CollectionPtr& meshes,
  const EntityRef& root,
  std::map<smtk::model::EntityRef, std::pair<smtk::model::EntityRef, smtk::mesh::MeshSet> >&
    entityrefMap,
  std::set<smtk::model::EntityRef>& touched)
{
  EntityRefArray children = (root.isModel()
      ? root.as<Model>().cellsAs<EntityRefArray>()
      : (root.isCellEntity()
            ? root.as<CellEntity>().boundingCellsAs<EntityRefArray>()
            : (root.isGroup() ? root.as<Group>().members<EntityRefArray>() : EntityRefArray())));
  if (root.isModel())
  {
    // Make sure groups are handled last to avoid unexpected "parents" in entityrefMap.
    EntityRefArray tmp;
    tmp = root.as<Model>().submodelsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
    tmp = root.as<Model>().groupsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
  }
  EntityRefs uniqueChildren;
  std::copy(children.begin(), children.end(), std::inserter(uniqueChildren, uniqueChildren.end()));
  for (EntityRefs::const_iterator it = uniqueChildren.begin(); it != uniqueChildren.end(); ++it)
  {
    if (touched.find(*it) == touched.end())
    {
      touched.insert(*it);

      smtk::mesh::MeshSet entMesh = meshes->findAssociatedMeshes(*it);
      if (!entMesh.is_empty())
      {
        entityrefMap[*it] = std::pair<smtk::model::EntityRef, smtk::mesh::MeshSet>(root, entMesh);
      }
      this->FindEntitiesWithMesh(meshes, *it, entityrefMap, touched);
    }
  }
}

/// Do the actual work of grabbing primitives from the model.
void vtkMeshMultiBlockSource::GenerateRepresentationFromMesh(vtkMultiBlockDataSet* mbds)
{

  if (this->MeshCollectionID && this->MeshCollectionID[0] && this->m_meshMgr)
  {
    smtk::common::UUID mcuid(this->MeshCollectionID);
    smtk::mesh::CollectionPtr meshcollect = this->m_meshMgr->collection(mcuid);
    Model modelEntity;
    bool modelRequiresNormals = false;
    if (this->ModelEntityID && this->ModelEntityID[0] && this->m_modelMgr)
    {
      smtk::common::UUID uid(this->ModelEntityID);
      smtk::model::EntityRef entity(this->m_modelMgr, uid);
      modelEntity = entity.isModel() ? entity.as<smtk::model::Model>() : entity.owningModel();
    }

    if (modelEntity.isValid() && meshcollect->isValid() && meshcollect->numberOfMeshes() > 0)
    {
      // See if the model has any instructions about
      // whether to generate surface normals.
      if (modelEntity.hasIntegerProperty("generate normals"))
      {
        const IntegerList& prop(modelEntity.integerProperty("generate normals"));
        if (!prop.empty() && prop[0])
          modelRequiresNormals = true;
      }

      // Map from entityref to its parent cell entity and its meshset
      std::map<smtk::model::EntityRef, std::pair<smtk::model::EntityRef, smtk::mesh::MeshSet> >
        entityrefMap;
      std::set<smtk::model::EntityRef> touched; // make this go out of scope soon.
      this->FindEntitiesWithMesh(meshcollect, modelEntity, entityrefMap, touched);

      mbds->SetNumberOfBlocks(static_cast<unsigned>(entityrefMap.size()));
      vtkIdType i;
      std::map<smtk::model::EntityRef,
        std::pair<smtk::model::EntityRef, smtk::mesh::MeshSet> >::iterator cit;
      for (i = 0, cit = entityrefMap.begin(); cit != entityrefMap.end(); ++cit, ++i)
      {
        vtkNew<vtkPolyData> poly;
        mbds->SetBlock(i, poly.GetPointer());
        // Set the block name to the entity UUID.
        mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), cit->first.name().c_str());
        this->GenerateRepresentationForSingleMesh(
          cit->second.second, poly.GetPointer(), cit->first, modelRequiresNormals);
        // std::cout << "UUID: " << (*cit).entity().toString().c_str() << " Block: " << i << std::endl;
        // as a convenient method to get the flat block index in multiblock
        if (!(cit->first.entity().isNull()))
        {
          internal_AddBlockInfo(meshcollect, cit->first, cit->second.second, cit->second.first, i,
            poly.GetPointer(), this->m_Meshset2BlockIdMap);
        }
      }

      // TODO: how do we handle submodels in a multiblock dataset? We could have
      //       a cycle in the submodels, so treating them as trees would not work.
      // Finally, if nothing has any tessellation information, see if any is associated
      // with the model itself.
    }
    else
    {
      std::cout << "Can not find the model: "
                << (this->ModelEntityID ? this->ModelEntityID : "NULL")
                << ", use all meshes from mesh collection: " << this->MeshCollectionID << std::endl;

      smtk::mesh::MeshSet allMeshes = meshcollect->meshes();
      mbds->SetNumberOfBlocks(static_cast<unsigned>(allMeshes.size()));

      for (std::size_t i = 0; i < allMeshes.size(); ++i)
      {
        vtkNew<vtkPolyData> poly;

        std::ostringstream defaultName;
        defaultName << "mesh " << i;
        smtk::mesh::MeshSet singleMesh = allMeshes.subset(i);
        std::vector<std::string> meshNames = singleMesh.names();
        std::string meshName = meshNames.size() > 0 ? meshNames[0] : defaultName.str();
        // Set the block name to a mesh name if it has one.
        // for now, use "mesh (<cell type>) <index>" for name
        mbds->GetMetaData(static_cast<unsigned>(i))
          ->Set(vtkCompositeDataSet::NAME(), meshName.c_str());
        this->GenerateRepresentationForSingleMesh(
          singleMesh, poly.GetPointer(), smtk::model::EntityRef(), modelRequiresNormals);
        mbds->SetBlock(static_cast<unsigned>(i), poly.GetPointer());
        smtk::model::EntityRefArray ents;
        bool validEnts = singleMesh.modelEntities(ents);
        if (validEnts && ents.size() > 0)
        {
          internal_AddBlockEntityInfo(
            singleMesh, ents[0], i, poly.GetPointer(), this->m_Meshset2BlockIdMap);
        }
      }
    }
  }
  else
  {
    vtkGenericWarningMacro(<< "A valid mesh collection id was not set, abort!");
  }
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkMeshMultiBlockSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo), vtkInformationVector* outInfo)
{
  this->m_Meshset2BlockIdMap.clear();
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!output)
  {
    vtkErrorMacro("No output dataset");
    return 0;
  }

  if (!this->m_meshMgr)
  {
    vtkErrorMacro("No input mesh manager");
    return 0;
  }

  // Destroy the cache if the parameters have changed since it was generated.
  if (this->CachedOutput && this->GetMTime() > this->CachedOutput->GetMTime())
    this->SetCachedOutput(NULL);

  if (!this->CachedOutput)
  { // Populate a polydata with tessellation information from the model.
    vtkNew<vtkMultiBlockDataSet> rep;
    this->GenerateRepresentationFromMesh(rep.GetPointer());
    this->SetCachedOutput(rep.GetPointer());
  }
  output->ShallowCopy(this->CachedOutput);
  return 1;
}

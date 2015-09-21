//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/vtkModelMultiBlockSource.h" // for Array name

#include "smtk/model/EntityRef.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
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
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"
#include "vtkPolyDataNormals.h"

#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>

using namespace smtk::model;

vtkStandardNewMacro(vtkMeshMultiBlockSource);
vtkCxxSetObjectMacro(vtkMeshMultiBlockSource,CachedOutput,vtkMultiBlockDataSet);

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
void vtkMeshMultiBlockSource::GetUUID2BlockIdMap(std::map<smtk::common::UUID, unsigned int>& uuid2mid)
{
  uuid2mid.clear();
  uuid2mid.insert(this->m_UUID2BlockIdMap.begin(), this->m_UUID2BlockIdMap.end());
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
/// 'Volume' field array to color by volume
static void internal_AddBlockInfo(smtk::model::ManagerPtr manager,
  const smtk::model::EntityRef& entityref, const smtk::model::EntityRef& bordantCell,
  const vtkIdType& blockId,
  vtkPolyData* poly, std::map<smtk::common::UUID, unsigned int>& uuid2BlockId)
{
  manager->setIntegerProperty(entityref.entity(), "block_index", blockId);
  uuid2BlockId[entityref.entity()] = static_cast<unsigned int>(blockId);

  // Add Entity UUID to fieldData
  vtkNew<vtkStringArray> uuidArray;
  uuidArray->SetNumberOfComponents(1);
  uuidArray->SetNumberOfTuples(1);
  uuidArray->SetName(vtkModelMultiBlockSource::GetEntityTagName());
  uuidArray->SetValue(0, entityref.entity().toString());
  poly->GetFieldData()->AddArray(uuidArray.GetPointer());

  smtk::model::EntityRefs vols;
  if(bordantCell.isValid() && bordantCell.isVolume())
    vols.insert(bordantCell);
 if(vols.size())
    {
    // Add volume UUID to fieldData
    vtkNew<vtkStringArray> volArray;
    volArray->SetNumberOfComponents(1);
    volArray->SetNumberOfTuples(vols.size());
    int ai = 0;
    for (smtk::model::EntityRefs::iterator it = vols.begin();
         it != vols.end(); ++it, ++ai)
      {
      volArray->SetValue(ai, (*it).entity().toString());
      }
    volArray->SetName(vtkModelMultiBlockSource::GetVolumeTagName());
    poly->GetFieldData()->AddArray(volArray.GetPointer());
    }
}

static void convert_smtkMesh_singleDimCells_to_vtkPolyData(const smtk::mesh::CellSet& cells,
  vtkPoints* pts, vtkPolyData* pd)
{
  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(cells,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

//  std::vector<boost::int64_t> conn( connectivityLength + numberOfCells );
//  std::vector<boost::int64_t> locations( numberOfCells );
//  std::vector<unsigned char> types( numberOfCells );
//  std::vector<double> dpoints( numberOfPoints * 3 );

  // cell connectivity
  vtkNew<vtkCellArray> cellarray;

  // points coordinates
  pts->SetDataTypeToDouble();

  if(numberOfPoints == 1)
    {
    double xyz[3];
    cells.points().get(xyz);
    pts->InsertNextPoint(xyz);
    vtkNew<vtkIdList> ptids;
    ptids->InsertNextId(0);
    cellarray->InsertNextCell(ptids.GetPointer());
    pd->SetVerts(cellarray.GetPointer());
    }
  else
    {
    pts->SetNumberOfPoints(numberOfPoints);
    pts->Allocate(numberOfPoints);
    double *rawPoints = static_cast<double*>(pts->GetVoidPointer(0));

    cellarray->Allocate(connectivityLength + numberOfCells);
    boost::int64_t* cellconn = reinterpret_cast<boost::int64_t *>(
                cellarray->WritePointer(numberOfCells, connectivityLength + numberOfCells));
    smtk::mesh::PreAllocatedTessellation tess(cellconn,
                                              rawPoints);

    smtk::mesh::extractTessellation(cells, tess);
    smtk::mesh::CellTypes ctypes = cells.types().cellTypes();
    // We are using highest dimension cells only
    if (ctypes[smtk::mesh::Triangle]
      || ctypes[smtk::mesh::Quad]
      || ctypes[smtk::mesh::Polygon]
      )
      {
      pd->SetPolys(cellarray.GetPointer());
      }
    else if (ctypes[smtk::mesh::Line])
      {
      pd->SetLines(cellarray.GetPointer());
      }
  /*
    else if (ctypes[smtk::mesh::Vertex])
      {
      pd->SetVerts(cellarray.GetPointer());
      }

    else if (ctypes[smtk::mesh::Tetrahedron]
      || ctypes[smtk::mesh::Pyramid]
      || ctypes[smtk::mesh::Wedge]
      || ctypes[smtk::mesh::Hexahedron]
      )
      {
      }
  */
    else
      {
      // any strips ??
      // pd->SetStrips(cellarray);
      }
    }
}

static void convert_smtkMesh_to_vtkPolyData(const smtk::mesh::MeshSet& entMesh,
  vtkPoints* pts, vtkPolyData* pd)
{
  // We are only getting the highest dimension cells starting Dims2
  smtk::mesh::CellSet cells = entMesh.cells(smtk::mesh::Dims2);

  if( cells.is_empty() == true)
    {
    cells = entMesh.cells(smtk::mesh::Dims1);
    }
  if( cells.is_empty() == true)
    {
    cells = entMesh.cells(smtk::mesh::Dims0);
    }

  convert_smtkMesh_singleDimCells_to_vtkPolyData(cells, pts, pd);
}

/// Loop over the model generating blocks of polydata.
void vtkMeshMultiBlockSource::GenerateNormals(
    vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals)
{
  if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
    {
    bool reallyNeedNormals = genNormals;
    if ( entityref.isValid() && entityref.hasIntegerProperty("generate normals"))
      { // Allow per-entity setting to override per-model setting
      const IntegerList& prop(
        entityref.integerProperty(
          "generate normals"));
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
void vtkMeshMultiBlockSource::GenerateRepresentationForSingleMesh(
  const smtk::mesh::MeshSet& meshes,
  vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals)
{
  vtkNew<vtkPoints> pts;
  pd->SetPoints(pts.GetPointer());

  if (!meshes.is_empty())
    {
/*
    std::cout << "single mesh, size=" << meshes.size() << ", cells=" << meshes.cells().size() << std::endl;
    smtk::mesh::CellTypes ctypes = meshes.cells().types().cellTypes();
    std::cout << "TypeSet hasCells=" << meshes.cells().types().hasCells() << ", cellTypes=" << ctypes.size() << std::endl;

    std::cout << "CellType vertex=" << ctypes[smtk::mesh::Vertex] << std::endl;
    std::cout << "CellType line=" << ctypes[smtk::mesh::Line] << std::endl;
    std::cout << "CellType triagle=" << ctypes[smtk::mesh::Triangle] << std::endl;
    std::cout << "CellType quad=" << ctypes[smtk::mesh::Quad] << std::endl;
    std::cout << "CellType polygon=" << ctypes[smtk::mesh::Polygon] << std::endl;
    std::cout << "CellType tet=" << ctypes[smtk::mesh::Tetrahedron] << std::endl;
    std::cout << "CellType pyramid=" << ctypes[smtk::mesh::Pyramid] << std::endl;
    std::cout << "CellType wedge=" << ctypes[smtk::mesh::Wedge] << std::endl;
    std::cout << "CellType hex=" << ctypes[smtk::mesh::Hexahedron] << std::endl;
*/
    //we want all 0d, 1d, 2d, and shells of 3d elments
    smtk::mesh::MeshSet shell = meshes.subset(smtk::mesh::Dims3).extractShell();
    smtk::mesh::MeshSet twoD = meshes.subset(smtk::mesh::Dims2);
    smtk::mesh::MeshSet oneD = meshes.subset(smtk::mesh::Dims1);
    smtk::mesh::MeshSet zeroD = meshes.subset(smtk::mesh::Dims0);

    smtk::mesh::MeshSet toRender = shell;
    toRender.append(twoD);
    toRender.append(oneD);
    toRender.append(zeroD);
        
    convert_smtkMesh_to_vtkPolyData(toRender, pts.GetPointer(), pd);
    
    this->GenerateNormals(pd, entityref, genNormals);
    }
}


/// Recursively find all the entities with tessellation
void vtkMeshMultiBlockSource::FindEntitiesWithMesh(
  const smtk::mesh::CollectionPtr& meshes,
  const CellEntity &cellent,
  std::map<smtk::model::EntityRef, smtk::model::EntityRef> &entityrefMap)
{
  CellEntities cellents = cellent.isModel() ?
    cellent.as<Model>().cells() : cellent.boundingCells();
  for (CellEntities::const_iterator it = cellents.begin(); it != cellents.end(); ++it)
    {
    if(!meshes->findAssociatedMeshes(*it).is_empty())
      {
      entityrefMap[*it] = cellent;
      }
    if((*it).boundingCells().size() > 0)
      {
      this->FindEntitiesWithMesh(meshes, *it, entityrefMap);
      }
    }
}

/// Do the actual work of grabbing primitives from the model.
void vtkMeshMultiBlockSource::GenerateRepresentationFromMesh(
  vtkMultiBlockDataSet* mbds)
{
  if(this->MeshCollectionID && this->MeshCollectionID[0] && this->m_meshMgr)
    {
    smtk::common::UUID mcuid(this->MeshCollectionID);
    smtk::mesh::CollectionPtr meshcollect = this->m_meshMgr->collection(mcuid);
    Model modelEntity;
    bool modelRequiresNormals = false;
    if (this->ModelEntityID && this->ModelEntityID[0] && this->m_modelMgr)
      {
      smtk::common::UUID uid(this->ModelEntityID);
      smtk::model::EntityRef entity(this->m_modelMgr, uid);
      modelEntity = entity.isModel() ?
        entity.as<smtk::model::Model>() : entity.owningModel();
      }
    if (modelEntity.isValid() && meshcollect->isValid() && meshcollect->numberOfMeshes() > 0)
      {
      // See if the model has any instructions about
      // whether to generate surface normals.
      if ( modelEntity.hasIntegerProperty("generate normals"))
        {
        const IntegerList& prop(modelEntity.integerProperty("generate normals"));
        if (!prop.empty() && prop[0])
          modelRequiresNormals = true;
        }

      // First, enumerate all free cells and their boundaries to
      // find those which provide tessellations.
      // smtk::model::EntityRefs entityrefs;
      // Map from entityref to its parent cell entity
      std::map<smtk::model::EntityRef, smtk::model::EntityRef> entityrefMap;

      this->FindEntitiesWithMesh(meshcollect, modelEntity, entityrefMap);

      Groups groups = modelEntity.groups();
      mbds->SetNumberOfBlocks(entityrefMap.size() + groups.size());
      vtkIdType i;
      std::map<smtk::model::EntityRef, smtk::model::EntityRef>::iterator cit;
      for (i = 0, cit = entityrefMap.begin(); cit != entityrefMap.end(); ++cit, ++i)
        {
        vtkNew<vtkPolyData> poly;
        mbds->SetBlock(i, poly.GetPointer());
        // Set the block name to the entity UUID.
        mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), cit->first.name().c_str());
        this->GenerateRepresentationForSingleMesh(meshcollect->findAssociatedMeshes(cit->first),
          poly.GetPointer(), cit->first, modelRequiresNormals);
        // std::cout << "UUID: " << (*cit).entity().toString().c_str() << " Block: " << i << std::endl;
        // as a convenient method to get the flat block index in multiblock
        if(!(cit->first.entity().isNull()))
          {
          internal_AddBlockInfo(this->m_modelMgr, cit->first, cit->second,
            i, poly.GetPointer(), this->m_UUID2BlockIdMap);
          }
        }

      // Now look at groups of the model to see if those have any tessellation data
      i = 0;
      for (Groups::iterator git = groups.begin(); git != groups.end(); ++git, ++i)
        {
        if (git->hasTessellation())
          {
          vtkNew<vtkPolyData> poly;
          mbds->SetBlock(entityrefMap.size() + i, poly.GetPointer());
          mbds->GetMetaData(entityrefMap.size() + i)->Set(vtkCompositeDataSet::NAME(), git->name().c_str());

          this->GenerateRepresentationForSingleMesh(meshcollect->findAssociatedMeshes(*git),
            poly.GetPointer(), *git, modelRequiresNormals);
          // as a convenient method to get the flat block_index in multiblock
          internal_AddBlockInfo(this->m_modelMgr, *git, modelEntity, entityrefMap.size() + i,
            poly.GetPointer(), this->m_UUID2BlockIdMap);
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

      smtk::mesh::MeshSet allMeshes = meshcollect->meshes( );
      mbds->SetNumberOfBlocks(allMeshes.size());

      std::cout << "Total meshes=" << allMeshes.size() << std::endl;
      std::cout << "Total cells=" << allMeshes.cells().size() << std::endl;

      for(std::size_t i=0; i<allMeshes.size(); ++i)
        {
        vtkNew<vtkPolyData> poly;
        mbds->SetBlock(i, poly.GetPointer());

        std::ostringstream defaultName;
        defaultName << "mesh " << i;
        smtk::mesh::MeshSet singleMesh = allMeshes.subset(i);
        std::vector< std::string > meshNames = singleMesh.names();
        std::string meshName = meshNames.size() > 0 ?
                               meshNames[0] :
                               defaultName.str();
        // Set the block name to a mesh name if it has one.
        // for now, use "mesh (<cell type>) <index>" for name
        mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), meshName.c_str());
        this->GenerateRepresentationForSingleMesh(
          singleMesh, poly.GetPointer(), smtk::model::EntityRef(), modelRequiresNormals);
        }
      }
    }
  else
    {
    vtkGenericWarningMacro(<< "A valid mesh collection id was not set, abort!");
    }
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkMeshMultiBlockSource::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* outInfo)
{
  this->m_UUID2BlockIdMap.clear();
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

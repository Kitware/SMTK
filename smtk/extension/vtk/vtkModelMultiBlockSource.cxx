//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/vtkModelMultiBlockSource.h"

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
#include "vtkRenderer.h"
#include "vtkRenderView.h"
#include "vtkStringArray.h"
#include "vtkPolyDataNormals.h"


using namespace smtk::model;

vtkStandardNewMacro(vtkModelMultiBlockSource);
vtkCxxSetObjectMacro(vtkModelMultiBlockSource,CachedOutput,vtkMultiBlockDataSet);

vtkModelMultiBlockSource::vtkModelMultiBlockSource()
{
  this->SetNumberOfInputPorts(0);
  this->CachedOutput = NULL;
  for (int i = 0; i < 4; ++i)
    {
    this->DefaultColor[i] = 1.;
    }
  this->ModelEntityID = NULL;
  this->AllowNormalGeneration = 0;
}

vtkModelMultiBlockSource::~vtkModelMultiBlockSource()
{
  this->SetCachedOutput(NULL);
  this->SetModelEntityID(NULL);
}

void vtkModelMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: " << this->ModelMgr.get() << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
  os << indent << "ModelEntityID: " << this->ModelEntityID << "\n";
  os << indent << "AllowNormalGeneration: " << this->AllowNormalGeneration << "\n";
}

/// Set the SMTK model to be displayed.
void vtkModelMultiBlockSource::SetModelManager(smtk::model::ManagerPtr model)
{
  if (this->ModelMgr == model)
    {
    return;
    }
  this->ModelMgr = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::ManagerPtr vtkModelMultiBlockSource::GetModelManager()
{
  return this->ModelMgr;
}

/// Get the map from model entity UUID to the block index in multiblock output
void vtkModelMultiBlockSource::GetUUID2BlockIdMap(std::map<smtk::common::UUID, unsigned int>& uuid2mid)
{
  uuid2mid.clear();
  uuid2mid.insert(this->UUID2BlockIdMap.begin(), this->UUID2BlockIdMap.end());
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkModelMultiBlockSource::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(NULL);
}

/*! \fn vtkModelMultiBlockSource::GetDefaultColor()
 *  \brief Get the RGBA color for model entities that do not have a color property set.
 *
 * The default value for DefaultColor is [1., 1., 1., 1.].
 *
 *  \fn vtkModelMultiBlockSource::GetDefaultColor(double& r, double& g, double& b, double& a)
 *  \brief Get the RGBA color for model entities that do not have a color property set.
 *
 * The default value for DefaultColor is [1., 1., 1., 1.].
 *
 *  \fn vtkModelMultiBlockSource::GetDefaultColor(double rgba[4])
 *  \brief Get the RGBA color for model entities that do not have a color property set.
 *
 * The default value for DefaultColor is [1., 1., 1., 1.].
 */

/*! \fn vtkModelMultiBlockSource::SetDefaultColor(double r, double g, double b, double a)
 *  \brief Set the default color (RGBA) for each model entity.
 *
 * This color will be assigned to each cell of each block for entities
 * that do not provide a color float-property.
 *
 * Setting the alhpa component of the default color to a
 * negative number will turn off per-cell color array generation
 * to save space.
 *
 * \sa vtkModelMultiBlockSource::GetDefaultColor
 *
 *  \fn vtkModelMultiBlockSource::SetDefaultColor(double rgba[4])
 *  \brief Set the default color (RGBA) for each model entity.
 *
 * This color will be assigned to each cell of each block for entities
 * that do not provide a color float-property.
 *
 * Setting the alhpa component of the default color to a
 * negative number will turn off per-cell color array generation
 * to save space.
 *
 * \sa vtkModelMultiBlockSource::GetDefaultColor
 */

static void AddEntityTessToPolyData(
  const smtk::model::EntityRef& entityref, vtkPoints* pts, vtkPolyData* pd)
{
  //gotMesh will get Analsyis mesh if it exists, and will fall back
  //to model tessellation if not.
  const smtk::model::Tessellation* tess = entityref.gotMesh();
  if (!tess)
    return;

  vtkIdType i;
  //vtkIdType connOffset = pts->GetNumberOfPoints();
  std::vector<vtkIdType> conn;
  std::string uuidStr = entityref.entity().toString();
  vtkIdType npts = tess->coords().size() / 3;
  for (i = 0; i < npts; ++i)
    {
    pts->InsertNextPoint(&tess->coords()[3*i]);
    }
  Tessellation::size_type off;
  vtkNew<vtkCellArray> verts;
  vtkNew<vtkCellArray> lines;
  vtkNew<vtkCellArray> polys;
  vtkNew<vtkCellArray> strip;
  bool have_verts = false;
  bool have_lines = false;
  bool have_polys = false;
  bool have_strip = false;
  for (off = tess->begin(); off != tess->end(); off = tess->nextCellOffset(off))
    {
    Tessellation::size_type cell_type = tess->cellType(off);
    Tessellation::size_type cell_shape = tess->cellShapeFromType(cell_type);
    std::vector<int> cell_conn;
    Tessellation::size_type num_verts = tess->vertexIdsOfCell(off, cell_conn);
    std::vector<vtkIdType> vtk_conn(cell_conn.begin(), cell_conn.end());
    switch (cell_shape)
      {
    case TESS_VERTEX:          have_verts = true; verts->InsertNextCell(1, &vtk_conn[0]); break;
    case TESS_TRIANGLE:        have_polys = true; polys->InsertNextCell(3, &vtk_conn[0]); break;
    case TESS_QUAD:            have_polys = true; polys->InsertNextCell(4, &vtk_conn[0]); break;
    case TESS_POLYVERTEX:      have_verts = true; verts->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_POLYLINE:        have_lines = true; lines->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_POLYGON:         have_polys = true; polys->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_TRIANGLE_STRIP:  have_strip = true; strip->InsertNextCell(num_verts, &vtk_conn[0]); break;
    default:
      std::cerr << "Invalid cell shape " << cell_shape << " at offset " << off << ". Skipping.\n";
      continue;
      break;
      }
    }
  if (have_verts) pd->SetVerts(verts.GetPointer());
  if (have_lines) pd->SetLines(lines.GetPointer());
  if (have_polys) pd->SetPolys(polys.GetPointer());
  if (have_strip) pd->SetStrips(strip.GetPointer());
}

/// Add customized block info.
/// Mapping from UUID to block id
/// Field data arrays
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
/*
  EntityRef embIn = entityref.embeddedIn();
  if(embIn.isValid() && embIn.isVolume())
      vols.insert(embIn);
  else
    {
    while(embIn.isValid())
      {
      embIn = embIn.embeddedIn();
      if(embIn.isValid() && embIn.isVolume())
        {
        vols.insert(embIn);
        break;
        }
      }
    }
*/
/*
  if(entityref.isEdge())
    {
    smtk::model::Edge eg = entityref.as<smtk::model::Edge>();
    for(smtk::model::EdgeUses::iterator it = eg.edgeUses().begin();
        it != eg.edgeUses().end(); ++it)
      {
      if((*it).faceUse().isValid())
        vols.push_back((*it).faceUse().volume());
      }
    }
  else if(entityref.isFace())
    {
    //vols = entityref.as<smtk::model::Face>().volumes(); // not working yet
    smtk::model::FaceUse fUse = entityref.as<smtk::model::Face>().negativeUse();
    if(fUse.isValid())
      vols.push_back(fUse.volume());
    fUse = entityref.as<smtk::model::Face>().positiveUse();
    if(fUse.isValid())
      vols.push_back(fUse.volume());
    }
*/
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

  // Add group UUID to fieldData
  vtkNew<vtkStringArray> groupArray;
  groupArray->SetNumberOfComponents(1);
  int na = entityref.numberOfArrangementsOfKind(SUBSET_OF);
  if(na > 0)
    {
    groupArray->SetNumberOfTuples(na);
    for (int i = 0; i < na; ++i)
      {
      groupArray->SetValue(i,
        entityref.relationFromArrangement(SUBSET_OF, i, 0).entity().toString());
      }
    groupArray->SetName(vtkModelMultiBlockSource::GetGroupTagName());
    poly->GetFieldData()->AddArray(groupArray.GetPointer());
    }
}

/// Loop over the model generating blocks of polydata.
void vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals)
{
  vtkNew<vtkPoints> pts;
  pd->SetPoints(pts.GetPointer());
  const smtk::model::Tessellation* tess;
  if (!(tess = entityref.hasTessellation()))
    { // Oops.
    return;
    }
  vtkIdType npts = tess->coords().size() / 3;
  pts->Allocate(npts);
  smtk::model::Entity* entity;
  if (entityref.isValid(&entity))
    {
    AddEntityTessToPolyData(entityref, pts.GetPointer(), pd);
    // Only create the color array if there is a valid default:
    if (this->DefaultColor[3] >= 0.)
      {
      FloatList rgba = entityref.color();
      vtkNew<vtkUnsignedCharArray> cellColor;
      cellColor->SetNumberOfComponents(4);
      cellColor->SetNumberOfTuples(pd->GetNumberOfCells());
      cellColor->SetName("Entity Color");
      for (int i = 0; i < 4; ++i)
        {
        cellColor->FillComponent(i,
          (rgba[3] >= 0 ? rgba[i] : this->DefaultColor[i])* 255.);
        }
      pd->GetCellData()->AddArray(cellColor.GetPointer());
      pd->GetCellData()->SetScalars(cellColor.GetPointer());
      }
    if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
      {
      bool reallyNeedNormals = genNormals;
      if ( entityref.hasIntegerProperty("generate normals"))
        { // Allow per-entity setting to override per-model setting
        const IntegerList& prop(
          entityref.integerProperty(
            "generate normals"));
        reallyNeedNormals = (!prop.empty() && prop[0]);
        }
      if (reallyNeedNormals)
        {
        this->NormalGenerator->SetInputDataObject(pd);
        this->NormalGenerator->Update();
        pd->ShallowCopy(this->NormalGenerator->GetOutput());
        }
      }
    }
}

/// Recursively find all the entities with tessellation
void vtkModelMultiBlockSource::FindEntitiesWithTessellation(
  const CellEntity &cellent,
  std::map<smtk::model::EntityRef, smtk::model::EntityRef> &entityrefMap)
{
  CellEntities cellents = cellent.isModel() ?
    cellent.as<Model>().cells() : cellent.boundingCells();
  for (CellEntities::const_iterator it = cellents.begin(); it != cellents.end(); ++it)
    {
    if((*it).hasTessellation())
      {
      entityrefMap[*it] = cellent;
      }
    if((*it).boundingCells().size() > 0)
      {
      this->FindEntitiesWithTessellation(*it, entityrefMap);
      }
    }
}

/// Do the actual work of grabbing primitives from the model.
void vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  vtkMultiBlockDataSet* mbds, smtk::model::ManagerPtr manager)
{
  if(this->ModelEntityID && this->ModelEntityID[0])
    {
    smtk::common::UUID uid(this->ModelEntityID);
    smtk::model::EntityRef entity(manager, uid);
    Model modelEntity = entity.isModel() ?
      entity.as<smtk::model::Model>() : entity.owningModel();
    if (modelEntity.isValid())
      {
      // See if the model has any instructions about
      // whether to generate surface normals.
      bool modelRequiresNormals = false;
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

      this->FindEntitiesWithTessellation(modelEntity, entityrefMap);

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
        this->GenerateRepresentationFromModel(poly.GetPointer(), cit->first, modelRequiresNormals);
        // std::cout << "UUID: " << (*cit).entity().toString().c_str() << " Block: " << i << std::endl;
        // as a convenient method to get the flat block index in multiblock
        if(!(cit->first.entity().isNull()))
          {
          internal_AddBlockInfo(manager, cit->first, cit->second, i, poly.GetPointer(), this->UUID2BlockIdMap);
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
          this->GenerateRepresentationFromModel(poly.GetPointer(), *git, modelRequiresNormals);
          // as a convenient method to get the flat block_index in multiblock
          internal_AddBlockInfo(manager, *git, modelEntity, entityrefMap.size() + i, poly.GetPointer(), this->UUID2BlockIdMap);
          }
        }
      // TODO: how do we handle submodels in a multiblock dataset? We could have
      //       a cycle in the submodels, so treating them as trees would not work.
      // Finally, if nothing has any tessellation information, see if any is associated
      // with the model itself.
      }
    else
      {
      vtkGenericWarningMacro(<< "Can not find the model entity with UUID: " << this->ModelEntityID);
      }
    }
  else
    {
    vtkGenericWarningMacro(<< "A valid model entity id was not set, so all tessellations are used.");

    mbds->SetNumberOfBlocks(manager->tessellations().size());
    vtkIdType i;
    smtk::model::UUIDWithTessellation it;
    for (i = 0, it = manager->tessellations().begin(); it != manager->tessellations().end(); ++it, ++i)
      {
      vtkNew<vtkPolyData> poly;
      mbds->SetBlock(i, poly.GetPointer());
      smtk::model::EntityRef entityref(manager, it->first);
      // Set the block name to the entity UUID.
      mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), entityref.name().c_str());
      this->GenerateRepresentationFromModel(poly.GetPointer(), entityref, this->AllowNormalGeneration);

      // as a convenient method to get the flat block_index in multiblock
      internal_AddBlockInfo(manager, entityref, smtk::model::EntityRef(), i, poly.GetPointer(), this->UUID2BlockIdMap);
      }
    }
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelMultiBlockSource::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* outInfo)
{
  this->UUID2BlockIdMap.clear();
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!output)
    {
    vtkErrorMacro("No output dataset");
    return 0;
    }

  if (!this->ModelMgr)
    {
    vtkErrorMacro("No input model");
    return 0;
    }

  if (!this->CachedOutput)
    { // Populate a polydata with tessellation information from the model.
    vtkNew<vtkMultiBlockDataSet> rep;
    this->GenerateRepresentationFromModel(rep.GetPointer(), this->ModelMgr);
    this->SetCachedOutput(rep.GetPointer());
    }
  output->ShallowCopy(this->CachedOutput);
  return 1;
}

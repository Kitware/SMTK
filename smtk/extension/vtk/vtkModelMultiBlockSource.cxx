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

#include "smtk/model/Cursor.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Tessellation.h"

#include "smtk/common/UUID.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
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
void vtkModelMultiBlockSource::GetUUID2BlockIdMap(std::map<std::string, unsigned int>& uuid2mid)
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
  const smtk::model::Cursor& cursor, vtkPoints* pts, vtkPolyData* pd, vtkStringArray* pedigree)
{
  const smtk::model::Tessellation* tess = cursor.hasTessellation();
  if (!tess)
    return;

  vtkIdType i;
  //vtkIdType connOffset = pts->GetNumberOfPoints();
  std::vector<vtkIdType> conn;
  std::string uuidStr = cursor.entity().toString();
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
    // WARNING!!!
    // Normally, it would matter what order the verts, lines, polys, and strips appear in...
    // but since the pedigree ID is the same for all the cells (and we assume for now that
    // there is one uuidStr per polydata), then it doesn't matter.
    pedigree->InsertNextValue(uuidStr);
    }
  if (have_verts) pd->SetVerts(verts.GetPointer());
  if (have_lines) pd->SetLines(lines.GetPointer());
  if (have_polys) pd->SetPolys(polys.GetPointer());
  if (have_strip) pd->SetStrips(strip.GetPointer());
}

/// Loop over the model generating blocks of polydata.
void vtkModelMultiBlockSource::GenerateRepresentationFromModelEntity(
  vtkPolyData* pd, const smtk::model::Cursor& cursor)
{
  vtkNew<vtkPoints> pts;
  vtkNew<vtkStringArray> pedigree;
  pedigree->SetName("UUID");
  pd->SetPoints(pts.GetPointer());
  pd->GetCellData()->SetPedigreeIds(pedigree.GetPointer());
  const smtk::model::Tessellation* tess;
  if (!(tess = cursor.hasTessellation()))
    { // Oops.
    return;
    }
  vtkIdType npts = tess->coords().size() / 3;
  pts->Allocate(npts);
  smtk::model::Entity* entity;
  if (cursor.isValid(&entity))
    {
    AddEntityTessToPolyData(cursor, pts.GetPointer(), pd, pedigree.GetPointer());
    // Only create the color array if there is a valid default:
    if (this->DefaultColor[3] >= 0.)
      {
      FloatList rgba = cursor.color();
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
    }
}

/// Recursively find all the entities with tessellation
void vtkModelMultiBlockSource::FindEntitiesWithTessellation(
  const CellEntities &cellents, smtk::model::Cursors &cursors)
{
  for (CellEntities::const_iterator it = cellents.begin(); it != cellents.end(); ++it)
    {
    if((*it).hasTessellation())
      {
      cursors.insert(*it);
      }
    else if((*it).boundingCells().size() > 0)
      {
      this->FindEntitiesWithTessellation((*it).boundingCells(), cursors);
      }
    }
}

/// Do the actual work of grabbing primitives from the model.
void vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  vtkMultiBlockDataSet* mbds, smtk::model::ManagerPtr manager)
{
  // TODO: how do we handle submodels in a multiblock dataset
  if(this->ModelEntityID && this->ModelEntityID[0])
    {
    smtk::common::UUID uid(this->ModelEntityID);
    smtk::model::Cursor entity(manager, uid);
    ModelEntity modelEntity = entity.isModelEntity() ?
      entity.as<smtk::model::ModelEntity>() : entity.owningModel();
    if (modelEntity.isValid())
      {
      smtk::model::Cursors cursors;
      CellEntities cellents = modelEntity.cells();
      this->FindEntitiesWithTessellation(cellents, cursors);

      mbds->SetNumberOfBlocks(cursors.size());
      vtkIdType i;
      smtk::model::Cursors::iterator cit;
      for (i = 0, cit = cursors.begin(); cit != cursors.end(); ++cit, ++i)
        {
        vtkNew<vtkPolyData> poly;
        mbds->SetBlock(i, poly.GetPointer());
        // Set the block name to the entity UUID.
        mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), (*cit).name().c_str());
        this->GenerateRepresentationFromModelEntity(poly.GetPointer(), *cit);
//        std::cout << "UUID: " << (*cit).entity().toString().c_str() << " Block: " << i << std::endl;
        this->UUID2BlockIdMap[(*cit).entity().toString()] = static_cast<unsigned int>(i);
        }
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
      smtk::model::Cursor cursor(manager, it->first);
      // Set the block name to the entity UUID.
      mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), cursor.name().c_str());
      this->GenerateRepresentationFromModelEntity(poly.GetPointer(), cursor);
      this->UUID2BlockIdMap[cursor.entity().toString()] = static_cast<unsigned int>(i);
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

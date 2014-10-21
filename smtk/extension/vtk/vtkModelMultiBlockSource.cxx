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

#include "smtk/model/CellEntity.h"
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

template<int Dim>
void AddEntityTessToPolyData(
  const smtk::model::Cursor& cursor, vtkPoints* pts, vtkCellArray* cells, vtkStringArray* pedigree)
{
  const smtk::model::Tessellation* tess = cursor.hasTessellation();
  if (!tess)
    return;

  vtkIdType i;
  vtkIdType connOffset = pts->GetNumberOfPoints();
  std::vector<vtkIdType> conn;
  std::string uuidStr = cursor.entity().toString();
  vtkIdType npts = tess->coords().size() / 3;
  for (i = 0; i < npts; ++i)
    {
    pts->InsertNextPoint(&tess->coords()[3*i]);
    }
  vtkIdType nconn = tess->conn().size();
  int ptsPerPrim = 0;
  if (nconn == 0 && Dim == 0)
    { // every point is a vertex cell.
    for (i = 0; i < npts; ++i)
      {
      vtkIdType pid = i + connOffset;
      cells->InsertNextCell(1, &pid);
      pedigree->InsertNextValue(uuidStr);
      }
    }
  else
    {
    for (i = 0; i < nconn; i += ptsPerPrim + 1)
      {
      if (Dim < 2)
        {
        ptsPerPrim = tess->conn()[i];
        }
      else
        {
        // TODO: Handle "extended" format that allows lines and verts.
        switch (tess->conn()[i] & 0x01) // bit 0 indicates quad, otherwise triangle.
          {
        case 0:
          ptsPerPrim = 3; //primType = VTK_TRIANGLE;
          break;
        case 1:
          ptsPerPrim = 4; //primType = VTK_QUAD;
          break;
        default:
            {
            vtkGenericWarningMacro(<< "Unknown tessellation primitive type: " << tess->conn()[i]);
            return;
            }
          }
        }
      if (nconn < (ptsPerPrim + i + 1))
        { // FIXME: Ignore junk at the end? Error message?
        break;
        }
      conn.resize(ptsPerPrim);
      // Rewrite connectivity for polydata:
      for (int k = 0; k < ptsPerPrim; ++k)
        {
        conn[k] = tess->conn()[i + k + 1] + connOffset;
        }
      cells->InsertNextCell(ptsPerPrim, &conn[0]);
      pedigree->InsertNextValue(uuidStr);
      }
    }
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
    switch(entity->dimension())
      {
    case 0:
        {
        vtkNew<vtkCellArray> verts;
        pd->SetVerts(verts.GetPointer());
        AddEntityTessToPolyData<0>(cursor, pts.GetPointer(), pd->GetVerts(), pedigree.GetPointer());
        }
      break;
    case 1:
        {
        vtkNew<vtkCellArray> lines;
        pd->SetLines(lines.GetPointer());
        AddEntityTessToPolyData<1>(cursor, pts.GetPointer(), pd->GetLines(), pedigree.GetPointer());
        }
      break;
    case 2:
        {
        vtkNew<vtkCellArray> polys;
        pd->SetPolys(polys.GetPointer());
        AddEntityTessToPolyData<2>(cursor, pts.GetPointer(), pd->GetPolys(), pedigree.GetPointer());
        }
      break;
    default:
      break;
      }
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
      for (CellEntities::iterator it = cellents.begin(); it != cellents.end(); ++it)
        {
        if((*it).hasTessellation())
          {
          cursors.insert(*it);
          }
        }
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

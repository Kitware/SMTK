//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/vtkModelSource.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"

#include "smtk/common/UUID.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderView.h"
#include "vtkStringArray.h"

using namespace smtk::model;
using namespace smtk::common;

vtkStandardNewMacro(vtkModelSource);
vtkCxxSetObjectMacro(vtkModelSource,CachedOutput,vtkPolyData);

vtkModelSource::vtkModelSource()
{
  this->SetNumberOfInputPorts(0);
  this->CachedOutput = NULL;
}

vtkModelSource::~vtkModelSource()
{
  this->SetCachedOutput(NULL);
}

void vtkModelSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Entities: " << this->Entities.size() << " entries\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
}

/// Set the SMTK model to be displayed.
void vtkModelSource::SetEntities(const smtk::model::EntityRefs& ents)
{
  if (this->Entities == ents)
    {
    return;
    }
  this->Entities = ents;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::EntityRefs vtkModelSource::GetEntities()
{
  return this->Entities;
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkModelSource::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(NULL);
}

static void AddEntityTessToPolyData(
  vtkPolyData* pd, vtkPoints* pts, vtkStringArray* pedigree,
  const smtk::common::UUID& uid, const Tessellation* tess)
{
  vtkIdType i;
  vtkIdType connOffset = pts->GetNumberOfPoints();
  std::vector<vtkIdType> conn;
  std::string uuidStr = uid.toString();
  vtkIdType npts = tess->coords().size() / 3;
  conn.reserve(npts);
  for (i = 0; i < npts; ++i)
    {
    conn.push_back(pts->InsertNextPoint(&tess->coords()[3*i]));
    }
  Tessellation::size_type off;
  vtkCellArray* verts = pd->GetVerts();
  vtkCellArray* lines = pd->GetLines();
  vtkCellArray* polys = pd->GetPolys();
  //vtkCellArray* strip = NULL; // TODO. Handle this case one day.
  for (off = tess->begin(); off != tess->end(); off = tess->nextCellOffset(off))
    {
    Tessellation::size_type cell_type = tess->cellType(off);
    Tessellation::size_type cell_shape = tess->cellShapeFromType(cell_type);
    std::vector<int> cell_conn;
    Tessellation::size_type num_verts = tess->vertexIdsOfCell(off, cell_conn);
    std::vector<vtkIdType> vtk_conn;
    vtk_conn.reserve(cell_conn.size());
    for (std::vector<int>::iterator connit = cell_conn.begin(); connit != cell_conn.end(); ++connit)
      vtk_conn.push_back(connOffset + *connit);
    switch (cell_shape)
      {
    case TESS_VERTEX:          verts->InsertNextCell(1, &vtk_conn[0]); break;
    case TESS_TRIANGLE:        polys->InsertNextCell(3, &vtk_conn[0]); break;
    case TESS_QUAD:            polys->InsertNextCell(4, &vtk_conn[0]); break;
    case TESS_POLYVERTEX:      verts->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_POLYLINE:        lines->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_POLYGON:         polys->InsertNextCell(num_verts, &vtk_conn[0]); break;
    //case TESS_TRIANGLE_STRIP:  strip->InsertNextCell(num_verts, &vtk_conn[0]); break;
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
}

/// Fetch children for model and group entities.
void vtkModelSource::AccumulateSortedEntities(
  EntityRefsByDim& accum, vtkIdType& npts, EntityRefs& toplevel)
{
  for (EntityRefs::const_iterator it = toplevel.begin(); it != toplevel.end(); ++it)
    {
    if (it->isModel())
      { // Add free cells
      CellEntities freeCells = it->as<Model>().cells();
      // Find all boundaries of all free cells
      CellEntities::iterator fcit;
      for (fcit = freeCells.begin(); fcit != freeCells.end(); ++fcit)
        {
        EntityRefs bdys = fcit->lowerDimensionalBoundaries(-1); // Get *all* boundaries.
        bdys.insert(*fcit); // include the bounding cell
        // Now call ourselves recursively so that we can get npts
        this->AccumulateSortedEntities(accum, npts, bdys);
        }
      }
    else if (it->isGroup())
      { // Add group members, but not their boundaries
      EntityRefs members = it->as<Group>().members<EntityRefs>();
      // Do this recursively since a group may contain other groups
      this->AccumulateSortedEntities(accum, npts, members);
      }
    else
      {
      const Tessellation* tess = it->hasTessellation();
      if (tess)
        {
        accum.insert(*it);
        npts += tess->coords().size() / 3; // TODO: assumes coords are 3-D.
        }
      }
    }
}

/// Do the actual work of grabbing primitives from the model.
void vtkModelSource::GenerateRepresentationFromModel(
  vtkPolyData* pd, const EntityRefsByDim& accum, vtkIdType npts)
{
  vtkNew<vtkPoints> pts;
  vtkNew<vtkStringArray> pedigree;
  pedigree->SetName("UUID");
  pts->Allocate(npts);
  pd->SetPoints(pts.GetPointer());
  pd->GetCellData()->SetPedigreeIds(pedigree.GetPointer());
  vtkNew<vtkCellArray> verts;
  vtkNew<vtkCellArray> lines;
  vtkNew<vtkCellArray> polys;
  pd->SetVerts(verts.GetPointer());
  pd->SetLines(lines.GetPointer());
  pd->SetPolys(polys.GetPointer());
  // TODO: Do not handle cells with different tessellation dimension
  //       than their cell type would otherwise indicate.
  //       For instance, no surface cell should contain a polyline or
  //       vertex tessellation entry. Pedigree IDs will be wrong in
  //       this case.
  // TODO: Do not handle strips currently since that would require
  //       us to sort entityrefs by whether their tessellations had any
  //       strips.
  EntityRefsByDim::const_iterator it;
  for (it = accum.begin(); it != accum.end(); ++it)
    {
    const Tessellation* tess = it->hasTessellation();
    if (!tess) continue;
    AddEntityTessToPolyData(pd, pts.GetPointer(), pedigree.GetPointer(), it->entity(), tess);
    }
}

/*
int vtkModelSource::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port < 2)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

int vtkModelSource::FillOutputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}
*/

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelSource::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* outInfo)
{
  vtkPolyData* output = vtkPolyData::GetData(outInfo, 0);
  if (!output)
    {
    vtkErrorMacro("No output dataset");
    return 0;
    }

  if (this->Entities.empty())
    { // Fail silently when no output requested.
    return 1;
    }

  if (!this->CachedOutput)
    { // Populate a polydata with tessellation information from the model.
    vtkNew<vtkPolyData> rep;
    EntityRefsByDim sortedEnts;
    vtkIdType npts = 0;
    this->AccumulateSortedEntities(sortedEnts, npts, this->Entities);
    this->GenerateRepresentationFromModel(rep.GetPointer(), sortedEnts, npts);
    this->SetCachedOutput(rep.GetPointer());
    }
  output->ShallowCopy(this->CachedOutput);
  return 1;
}

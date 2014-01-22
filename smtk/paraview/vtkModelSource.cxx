#include "smtk/paraview/vtkModelSource.h"

#include "smtk/model/Storage.h"
#include "smtk/model/Tessellation.h"

#include "smtk/util/UUID.h"

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
vtkInstantiatorNewMacro(vtkModelSource);

namespace smtk {
  namespace model {

vtkCxxSetObjectMacro(vtkModelSource,CachedOutput,vtkPolyData);

vtkModelSource* vtkModelSource::New()
{
  VTK_STANDARD_NEW_BODY(vtkModelSource);
}

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

  os << indent << "Model: " << this->Model.get() << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
}

/// Set the SMTK model to be displayed.
void vtkModelSource::SetModel(smtk::model::StoragePtr model)
{
  if (this->Model == model)
    {
    return;
    }
  this->Model = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::StoragePtr vtkModelSource::GetModel()
{
  return this->Model;
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkModelSource::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(NULL);
}

template<int Dim>
void AddEntityTessToPolyData(
  smtk::model::StoragePtr model, const smtk::util::UUID& uid, vtkPoints* pts, vtkCellArray* cells, vtkStringArray* pedigree)
{
  smtk::model::UUIDWithTessellation it = model->tessellations().find(uid);
  if (it == model->tessellations().end())
    {
    return;
    }
  vtkIdType i;
  vtkIdType connOffset = pts->GetNumberOfPoints();
  std::vector<vtkIdType> conn;
  std::string uuidStr = uid.toString();
  vtkIdType npts = it->second.coords().size() / 3;
  for (i = 0; i < npts; ++i)
    {
    pts->InsertNextPoint(&it->second.coords()[3*i]);
    }
  vtkIdType nconn = it->second.conn().size();
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
        ptsPerPrim = it->second.conn()[i];
        }
      else
        {
        // TODO: Handle "extended" format that allows lines and verts.
        switch (it->second.conn()[i] & 0x01) // bit 0 indicates quad, otherwise triangle.
          {
        case 0:
          ptsPerPrim = 3; //primType = VTK_TRIANGLE;
          break;
        case 1:
          ptsPerPrim = 4; //primType = VTK_QUAD;
          break;
        default:
            {
            vtkGenericWarningMacro(<< "Unknown tessellation primitive type: " << it->second.conn()[i]);
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
        conn[k] = it->second.conn()[i + k + 1] + connOffset;
        }
      cells->InsertNextCell(ptsPerPrim, &conn[0]);
      pedigree->InsertNextValue(uuidStr);
      }
    }
}

/// Do the actual work of grabbing primitives from the model.
void vtkModelSource::GenerateRepresentationFromModel(
  vtkPolyData* pd, smtk::model::StoragePtr model)
{
  vtkNew<vtkPoints> pts;
  vtkNew<vtkStringArray> pedigree;
  pedigree->SetName("UUID");
  pd->SetPoints(pts.GetPointer());
  pd->GetCellData()->SetPedigreeIds(pedigree.GetPointer());
  smtk::model::UUIDWithTessellation it;
  vtkIdType npts = 0;
  smtk::util::UUIDs modelVerts;
  smtk::util::UUIDs modelLines;
  smtk::util::UUIDs modelPolys;
  for (it = model->tessellations().begin(); it != model->tessellations().end(); ++it)
    {
    npts += it->second.coords().size() / 3;
    smtk::model::Entity* entity = model->findEntity(it->first);
    if (entity)
      {
      switch(entity->dimension())
        {
      case 0:
        modelVerts.insert(it->first);
        break;
      case 1:
        modelLines.insert(it->first);
        break;
      case 2:
        modelPolys.insert(it->first);
        break;
      default:
        if (it->second.conn().empty())
          {
          modelVerts.insert(it->first);
          }
        else if (it->second.conn()[0] > 0)
          { // assume everything that has a 0 entry is a triangle. (Three.JS format without quads or extra per-vertex stuff)
          modelPolys.insert(it->first);
          }
        else
          { // otherwise, it's a polyline
          modelLines.insert(it->first);
          }
        break;
        }
      }
    }
  pts->Allocate(npts);
  smtk::util::UUIDs::iterator uit;
  if (!modelVerts.empty())
    {
    vtkNew<vtkCellArray> verts;
    pd->SetVerts(verts.GetPointer());
    for (uit = modelVerts.begin(); uit != modelVerts.end(); ++uit)
      {
      AddEntityTessToPolyData<0>(model, *uit, pts.GetPointer(), pd->GetVerts(), pedigree.GetPointer());
      }
    }
  if (!modelLines.empty())
    {
    vtkNew<vtkCellArray> lines;
    pd->SetLines(lines.GetPointer());
    for (uit = modelLines.begin(); uit != modelLines.end(); ++uit)
      {
      AddEntityTessToPolyData<1>(model, *uit, pts.GetPointer(), pd->GetLines(), pedigree.GetPointer());
      }
    }
  if (!modelPolys.empty())
    {
    vtkNew<vtkCellArray> polys;
    pd->SetPolys(polys.GetPointer());
    for (uit = modelPolys.begin(); uit != modelPolys.end(); ++uit)
      {
      AddEntityTessToPolyData<2>(model, *uit, pts.GetPointer(), pd->GetPolys(), pedigree.GetPointer());
      }
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

  if (!this->Model)
    {
    vtkErrorMacro("No input model");
    return 0;
    }

  if (!this->CachedOutput)
    { // Populate a polydata with tessellation information from the model.
    vtkNew<vtkPolyData> rep;
    this->GenerateRepresentationFromModel(rep.GetPointer(), this->Model);
    this->SetCachedOutput(rep.GetPointer());
    }
  output->ShallowCopy(this->CachedOutput);
  return 1;
}

  } // namespace model
} // namespace smtk

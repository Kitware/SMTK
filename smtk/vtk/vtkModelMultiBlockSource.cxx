#include "smtk/vtk/vtkModelMultiBlockSource.h"

#include "smtk/model/Storage.h"
#include "smtk/model/Tessellation.h"

#include "smtk/util/UUID.h"

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

namespace smtk {
  namespace model {

vtkStandardNewMacro(vtkModelMultiBlockSource);
vtkCxxSetObjectMacro(vtkModelMultiBlockSource,CachedOutput,vtkMultiBlockDataSet);

vtkModelMultiBlockSource::vtkModelMultiBlockSource()
{
  this->SetNumberOfInputPorts(0);
  this->CachedOutput = NULL;
}

vtkModelMultiBlockSource::~vtkModelMultiBlockSource()
{
  this->SetCachedOutput(NULL);
}

void vtkModelMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: " << this->Model.get() << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
}

/// Set the SMTK model to be displayed.
void vtkModelMultiBlockSource::SetModel(smtk::model::StoragePtr model)
{
  if (this->Model == model)
    {
    return;
    }
  this->Model = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::StoragePtr vtkModelMultiBlockSource::GetModel()
{
  return this->Model;
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkModelMultiBlockSource::Dirty()
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

/// Loop over the model generating blocks of polydata.
void vtkModelMultiBlockSource::GenerateRepresentationFromModelEntity(
  vtkPolyData* pd, smtk::model::StoragePtr model, const smtk::util::UUID& uid)
{
  vtkNew<vtkPoints> pts;
  vtkNew<vtkStringArray> pedigree;
  pedigree->SetName("UUID");
  pd->SetPoints(pts.GetPointer());
  pd->GetCellData()->SetPedigreeIds(pedigree.GetPointer());
  smtk::model::UUIDWithTessellation it = model->tessellations().find(uid);
  if (it == model->tessellations().end())
    { // Oops.
    return;
    }
  vtkIdType npts = it->second.coords().size() / 3;
  pts->Allocate(npts);
  smtk::model::Entity* entity = model->findEntity(it->first);
  if (entity)
    {
    switch(entity->dimension())
      {
    case 0:
        {
        vtkNew<vtkCellArray> verts;
        pd->SetVerts(verts.GetPointer());
        AddEntityTessToPolyData<0>(model, it->first, pts.GetPointer(), pd->GetVerts(), pedigree.GetPointer());
        }
      break;
    case 1:
        {
        vtkNew<vtkCellArray> lines;
        pd->SetLines(lines.GetPointer());
        AddEntityTessToPolyData<1>(model, it->first, pts.GetPointer(), pd->GetLines(), pedigree.GetPointer());
        }
      break;
    case 2:
        {
        vtkNew<vtkCellArray> polys;
        pd->SetPolys(polys.GetPointer());
        AddEntityTessToPolyData<2>(model, it->first, pts.GetPointer(), pd->GetPolys(), pedigree.GetPointer());
        }
      break;
    default:
      break;
      }
    }
}

/// Do the actual work of grabbing primitives from the model.
void vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  vtkMultiBlockDataSet* mbds, smtk::model::StoragePtr model)
{
  mbds->SetNumberOfBlocks(model->tessellations().size());
  vtkIdType i;
  smtk::model::UUIDWithTessellation it;
  for (i = 0, it = model->tessellations().begin(); it != model->tessellations().end(); ++it, ++i)
    {
    vtkNew<vtkPolyData> poly;
    mbds->SetBlock(i, poly.GetPointer());
    // Set the block name to the entity UUID.
    mbds->GetMetaData(i)->Set(
      vtkCompositeDataSet::NAME(), it->first.toString().c_str());
    this->GenerateRepresentationFromModelEntity(poly.GetPointer(), model, it->first);
    }
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelMultiBlockSource::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* outInfo)
{
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
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
    vtkNew<vtkMultiBlockDataSet> rep;
    this->GenerateRepresentationFromModel(rep.GetPointer(), this->Model);
    this->SetCachedOutput(rep.GetPointer());
    }
  output->ShallowCopy(this->CachedOutput);
  return 1;
}

  } // namespace model
} // namespace smtk

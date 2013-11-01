#include "smtk/vtk/vtkSMTKModelRepresentation.h"

#include "smtk/model/ModelBody.h"
#include "smtk/model/Tessellation.h"

#include "smtk/util/UUID.h"

#include "vtkActor.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPoints.h"
#include "vtkRenderer.h"
#include "vtkRenderView.h"
#include "vtkStringArray.h"
#include "vtkTransformFilter.h"
#include "vtkViewTheme.h"

vtkStandardNewMacro(vtkSMTKModelRepresentation);
vtkCxxSetObjectMacro(vtkSMTKModelRepresentation,CachedOutput,vtkPolyData);
vtkCxxSetObjectMacro(vtkSMTKModelRepresentation,Actor,vtkActor);
vtkCxxSetObjectMacro(vtkSMTKModelRepresentation,Mapper,vtkPolyDataMapper);
vtkCxxSetObjectMacro(vtkSMTKModelRepresentation,Transform,vtkTransformFilter);

vtkSMTKModelRepresentation::vtkSMTKModelRepresentation()
{
  this->SetNumberOfInputPorts(0);

  this->CachedOutput = NULL;
  this->Transform = NULL;
  this->Mapper = NULL;
  this->Actor = NULL;

  vtkNew<vtkActor> act;
  vtkNew<vtkPolyDataMapper> map;
  vtkNew<vtkTransformFilter> tfm;

  this->SetTransform(tfm.GetPointer());
  this->SetMapper(map.GetPointer());
  this->SetActor(act.GetPointer());

  this->Actor->SetMapper(this->Mapper);
  this->Mapper->SetInputConnection(this->Transform->GetOutputPort());
}

vtkSMTKModelRepresentation::~vtkSMTKModelRepresentation()
{
  this->SetCachedOutput(NULL);
  this->SetTransform(NULL);
  this->SetMapper(NULL);
  this->SetActor(NULL);
}

void vtkSMTKModelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: " << this->Model.get() << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
  os << indent << "Actor: " << this->Actor << "\n";
  os << indent << "Mapper: " << this->Mapper << "\n";
  os << indent << "Transform: " << this->Transform << "\n";
}

/// Set the SMTK model to be displayed.
void vtkSMTKModelRepresentation::SetModel(smtk::model::ModelBodyPtr model)
{
  if (this->Model == model)
    {
    return;
    }
  this->Model = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::ModelBodyPtr vtkSMTKModelRepresentation::GetModel()
{
  return this->Model;
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkSMTKModelRepresentation::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(NULL);
}

/// Apply a theme to this representation.
void vtkSMTKModelRepresentation::ApplyViewTheme(vtkViewTheme* theme)
{
  (void)theme;
}

/// Do the actual work of grabbing primitives from the model.
void vtkSMTKModelRepresentation::GenerateRepresentationFromModel(
  vtkPolyData* pd, smtk::model::ModelBodyPtr model)
{
  vtkNew<vtkPoints> pts;
  vtkNew<vtkCellArray> polys;
  vtkNew<vtkStringArray> pedigree;
  pedigree->SetName("UUID");
  pd->SetPoints(pts.GetPointer());
  pd->SetPolys(polys.GetPointer());
  pd->GetCellData()->SetPedigreeIds(pedigree.GetPointer());
  vtkIdType i;
  smtk::model::UUIDWithTessellation it;
  vtkIdType npts = 0;
  vtkIdType nconn = 0;
  for (it = model->tessellations().begin(); it != model->tessellations().end(); ++it)
    {
    npts += it->second.coords.size() / 3;
    nconn += it->second.conn.size();
    }
  pts->SetNumberOfPoints(npts);
  vtkIdType curPt = 0;
  std::vector<vtkIdType> conn;
  for (it = model->tessellations().begin(); it != model->tessellations().end(); ++it)
    {
    std::string uuidStr = it->first.toString();
    // Each tessellation has connectivity relative to its local points.
    vtkIdType connOffset = curPt;

    npts = it->second.coords.size() / 3;
    for (i = 0; i < npts; ++i, ++curPt)
      {
      pts->SetPoint(curPt, &it->second.coords[3*i]);
      }
    nconn = it->second.conn.size();
    int ptsPerPrim = 0;
    int primType;
    for (i = 0; i < nconn; i += ptsPerPrim + 1)
      {
      // TODO: Handle "extended" format that allows lines and verts.
      switch (it->second.conn[i] & 0x01) // bit 0 indicates quad, otherwise triangle.
        {
      case 0:
        ptsPerPrim = 3;
        primType = VTK_TRIANGLE;
        break;
      case 1:
        ptsPerPrim = 4;
        primType = VTK_QUAD;
        break;
      default:
          {
          vtkErrorMacro(<< "Unknown tessellation primitive type: " << it->second.conn[i]);
          return;
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
        conn[k] = it->second.conn[i + k + 1] + connOffset;
        }
      pd->InsertNextCell(primType, ptsPerPrim, &conn[0]);
      pedigree->InsertNextValue(uuidStr);
      }
    }
}

/// Indicate that this representation generates polydata
int vtkSMTKModelRepresentation::FillOutputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkSMTKModelRepresentation::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* vtkNotUsed(outInfo))
{
  return 1;
}

void vtkSMTKModelRepresentation::PrepareForRendering(vtkRenderView* view)
{
  this->Superclass::PrepareForRendering(view);

  if (!this->Model)
    {
    vtkErrorMacro("No input");
    return;
    }

  if (!this->CachedOutput)
    { // Populate a polydata with tessellation information from the model.
    vtkNew<vtkPolyData> rep;
    this->GenerateRepresentationFromModel(rep.GetPointer(), this->Model);
    this->SetCachedOutput(rep.GetPointer());
    this->Transform->SetInputDataObject(this->CachedOutput);
    }
  this->Transform->SetTransform(view->GetTransform());
}

bool vtkSMTKModelRepresentation::AddToView(vtkView* view)
{
  vtkRenderView* rview = vtkRenderView::SafeDownCast(view);
  if (!rview)
    {
    vtkErrorMacro(<< "Cannot add to \"" << view->GetClassName() << "\"; must add to vtkRenderView.");
    return false;
    }
  rview->GetRenderer()->AddActor(this->Actor);
  return true;
}

bool vtkSMTKModelRepresentation::RemoveFromView(vtkView* view)
{
  vtkRenderView* rview = vtkRenderView::SafeDownCast(view);
  if (!rview)
    {
    vtkErrorMacro(<< "Cannot remove from \"" << view->GetClassName() << "\"; must remove from vtkRenderView.");
    return false;
    }
  rview->GetRenderer()->RemoveActor(this->Actor);
  return true;
}

vtkSelection* vtkSMTKModelRepresentation::ConvertSelection(vtkView* view, vtkSelection* selection)
{
  (void)view;
  return selection;
}

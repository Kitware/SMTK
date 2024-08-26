//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/widgets/vtkMeshInspectorRepresentation.h"
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

// SMTK headers
#include "smtk/common/UUID.h"

// ParaView headers
#include "vtkPVGeometryFilter.h"
#include "vtkPVMetaSliceDataSet.h"

// VTK headers
#include "vtkActor.h"
#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkBoundingBox.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkDataSet.h"
#include "vtkExtractBlock.h"
#include "vtkExtractEdges.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneCutter.h"
#include "vtkPropCollection.h"
#include "vtkProperty.h"
#include "vtkViewport.h"

#include <array>

namespace
{
// An empty input to use in case we are passed a null dataset.
vtkNew<vtkMultiBlockDataSet> g_DummyInput;
} // namespace

vtkStandardNewMacro(vtkMeshInspectorRepresentation);

vtkMeshInspectorRepresentation::vtkMeshInspectorRepresentation()
{
  // Configure the filters
  this->Extract->SetInputDataObject(
    g_DummyInput); // Do our best to avoid "no input" error messages.
  this->Extract->MaintainStructureOn();
  this->Crinkle->PreserveInputCells(1);
  this->Crinkle->SetImplicitFunction(this->GetUnderlyingPlane());
  this->Crinkle->SetDataSetCutFunction(this->GetUnderlyingPlane());
  this->Crinkle->SetInputConnection(this->Extract->GetOutputPort());
  this->Cutter->SetPlane(this->GetUnderlyingPlane());
  // this->Cutter->GenerateCutScalarsOn();
  this->Cutter->SetInputConnection(this->Extract->GetOutputPort());
  this->Surface->SetUseOutline(0);
  this->Surface->SetGenerateFeatureEdges(false);
  this->Surface->PassThroughCellIdsOn();
  this->Surface->PassThroughPointIdsOn();
  // this->Surface->SetInputConnection(this->Crinkle->GetOutputPort());
  this->Surface->SetInputConnection(this->Cutter->GetOutputPort());
  this->SurfaceMapper->SetInputConnection(this->Surface->GetOutputPort());
  this->SurfaceMapper->SetInputConnection(this->Surface->GetOutputPort());
  this->SurfaceActor->SetMapper(this->SurfaceMapper);
  this->Edges->SetInputConnection(this->Surface->GetOutputPort());
  this->EdgeMapper->SetInputConnection(this->Edges->GetOutputPort());
  this->EdgeMapper->ScalarVisibilityOff();
  this->EdgeActor->SetMapper(this->EdgeMapper);
}

vtkMeshInspectorRepresentation::~vtkMeshInspectorRepresentation() = default;

void vtkMeshInspectorRepresentation::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  auto nextIndent = indent.GetNextIndent();
  os << indent << "EdgeMapper:\n";
  this->EdgeMapper->PrintSelf(os, nextIndent);
  os << indent << "EdgeActor:\n";
  this->EdgeActor->PrintSelf(os, nextIndent);
  os << indent << "SurfaceMapper:\n";
  this->SurfaceMapper->PrintSelf(os, nextIndent);
  os << indent << "SurfaceActor:\n";
  this->SurfaceActor->PrintSelf(os, nextIndent);
  os << indent << "ExtractNeedsUpdate: " << (this->ExtractNeedsUpdate ? "Y\n" : "N\n");
  os << indent << "Components: " << this->Components.size() << "\n";
  os << indent << "Input: " << this->Input << "\n";
}

void vtkMeshInspectorRepresentation::SetInputConnection(vtkAlgorithmOutput* port)
{
  if (port)
  {
    auto* dataObject = port->GetProducer()->GetOutputDataObject(port->GetIndex());
    auto* input = dynamic_cast<vtkCompositeDataSet*>(dataObject);
    this->SetInput(input);
  }
}

void vtkMeshInspectorRepresentation::SetInput(vtkCompositeDataSet* input)
{
  if (input == this->Input)
  {
    return;
  }
  this->Input = input;
  this->Modified();
  this->Components.clear();
  this->Extract->RemoveAllIndices();
  this->ExtractNeedsUpdate = false;
  this->Extract->SetInputDataObject(this->Input ? this->Input.Get() : g_DummyInput.Get());
  // Debug:
  // this->Crinkle->SetInputDataObject(this->Input ? this->Input.Get() : g_DummyInput.Get());
}

vtkCompositeDataSet* vtkMeshInspectorRepresentation::GetInput()
{
  return this->Input;
}

bool vtkMeshInspectorRepresentation::AddId(const int* idData)
{
  const auto* bytes = reinterpret_cast<const uint8_t*>(idData);
  smtk::common::UUID uid(bytes, bytes + smtk::common::UUID::SIZE);
  bool didInsert = this->Components.insert(uid).second;
  if (didInsert)
  {
    this->ExtractNeedsUpdate = true;
    this->Modified();
  }
  return didInsert;
}

bool vtkMeshInspectorRepresentation::RemoveId(const int* idData)
{
  const auto* bytes = reinterpret_cast<const uint8_t*>(idData);
  smtk::common::UUID uid(bytes, bytes + smtk::common::UUID::SIZE);
  bool didRemove = this->Components.erase(uid) > 0;
  if (didRemove)
  {
    this->ExtractNeedsUpdate = true;
    this->Modified();
  }
  return didRemove;
}

void vtkMeshInspectorRepresentation::ResetIds()
{
  if (!this->Components.empty())
  {
    this->Components.clear();
    this->ExtractNeedsUpdate = true;
    this->Modified();
  }
}

void vtkMeshInspectorRepresentation::SetDrawHandles(bool drawHandles)
{
  if (this->DrawHandles == drawHandles)
  {
    return;
  }
  this->DrawHandles = drawHandles;
  this->Modified();
}

bool vtkMeshInspectorRepresentation::GetDrawHandles() const
{
  return this->DrawHandles;
}

void vtkMeshInspectorRepresentation::SetSliceType(const std::string& sliceType)
{
  if (sliceType == "Crinkle")
  {
    this->Surface->SetInputConnection(this->Crinkle->GetOutputPort());
  }
  else // (sliceType == "Flat")
  {
    this->Surface->SetInputConnection(this->Cutter->GetOutputPort());
  }
}

void vtkMeshInspectorRepresentation::SetInputArrayToProcess(
  int idx,
  int port,
  int connection,
  int fieldAssociation,
  const char* name)
{
  (void)idx;
  (void)port;
  (void)connection;
  std::string arrayName(name);
  if (arrayName.empty() || arrayName == "Solid Color")
  {
    this->SurfaceMapper->ScalarVisibilityOff();
  }
  else
  {
    this->SurfaceMapper->ScalarVisibilityOn();
    this->SurfaceMapper->SetColorModeToMapScalars();
    switch (fieldAssociation)
    {
      case vtkDataObject::POINT:
        this->SurfaceMapper->SetScalarModeToUsePointFieldData();
        break;
      case vtkDataObject::CELL:
        this->SurfaceMapper->SetScalarModeToUseCellFieldData();
        break;
      case vtkDataObject::FIELD:
        this->SurfaceMapper->SetScalarModeToUseFieldData();
        break;
      default:
        vtkWarningMacro("Unhandled field association " << fieldAssociation << "\n");
        break;
    }
    this->SurfaceMapper->SelectColorArray(name);
    this->SurfaceMapper->InterpolateScalarsBeforeMappingOn();
    if (fieldAssociation == vtkDataObject::FIELD)
    {
      this->SurfaceMapper->SetFieldDataTupleId(0); // TODO
    }
    // This appears not to work, but would be nice...
    // this->SurfaceMapper->SetInputArrayToProcess(idx, port, connection, fieldAssociation, name);
  }
}

vtkInformation* vtkMeshInspectorRepresentation::GetInputArrayInformation(int idx)
{
  return this->SurfaceMapper->GetInputArrayInformation(idx);
}

void vtkMeshInspectorRepresentation::SetSliceColorComponent(int comp)
{
  this->SurfaceMapper->SetArrayComponent(comp);
}

int vtkMeshInspectorRepresentation::GetSliceColorComponent()
{
  return this->SurfaceMapper->GetArrayComponent();
}

void vtkMeshInspectorRepresentation::SetSliceLookupTable(vtkScalarsToColors* lkup)
{
  this->SurfaceMapper->SetLookupTable(lkup);
}

vtkScalarsToColors* vtkMeshInspectorRepresentation::GetSliceLookupTable()
{
  return this->SurfaceMapper->GetLookupTable();
}

void vtkMeshInspectorRepresentation::SetSliceEdgeVisibility(bool visible)
{
  this->EdgeActor->SetVisibility(visible);
}

bool vtkMeshInspectorRepresentation::GetSliceEdgeVisibility()
{
  return this->EdgeActor->GetVisibility();
}

void vtkMeshInspectorRepresentation::SetSliceEdgeColor(double r, double g, double b, double a)
{
  this->EdgeActor->GetProperty()->SetDiffuseColor(r, g, b);
  this->EdgeActor->GetProperty()->SetOpacity(a);
}

double* vtkMeshInspectorRepresentation::GetSliceEdgeColor()
{
  this->EdgeActor->GetProperty()->GetDiffuseColor(this->EdgeColor.data());
  this->EdgeColor[3] = this->EdgeActor->GetProperty()->GetOpacity();
  return this->EdgeColor.data();
}

double* vtkMeshInspectorRepresentation::GetBounds()
{
  // Get parent representation's bounds
  vtkBoundingBox bbox(this->Superclass::GetBounds());
  // Add crinkle-surface bounds
  vtkDataObject* data = this->Surface->GetOutputDataObject(0);
  if (auto* composite = dynamic_cast<vtkCompositeDataSet*>(data))
  {
    composite->GetBounds(this->BoundsData.data());
    bbox.AddBounds(this->BoundsData.data());
  }
  else if (auto* dataset = dynamic_cast<vtkDataSet*>(data))
  {
    dataset->GetBounds(this->BoundsData.data());
    bbox.AddBounds(this->BoundsData.data());
  }
  else if (data)
  {
    vtkWarningMacro("Unhandled data object type \"" << data->GetClassName() << "\"");
  }
  bbox.GetBounds(this->BoundsData.data());
  return this->BoundsData.data();
}

void vtkMeshInspectorRepresentation::GetActors(vtkPropCollection* pc)
{
  this->Superclass::GetActors(pc);
  pc->AddItem(this->SurfaceActor);
  pc->AddItem(this->EdgeActor);
}

void vtkMeshInspectorRepresentation::ReleaseGraphicsResources(vtkWindow* window)
{
  this->Superclass::ReleaseGraphicsResources(window);
}

int vtkMeshInspectorRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->DrawHandles)
  {
    // The superclass calls BuildRepresentation:
    count += this->Superclass::RenderOpaqueGeometry(viewport);
  }
  else
  {
    this->BuildRepresentation();
  }
  // Now render our opaque geometry:
  count += this->SurfaceActor->RenderOpaqueGeometry(viewport);
  if (this->EdgeActor->GetVisibility())
  {
    count += this->EdgeActor->RenderOpaqueGeometry(viewport);
  }
  return count;
}

int vtkMeshInspectorRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->DrawHandles)
  {
    // The superclass calls BuildRepresentation:
    count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  }
  else
  {
    this->BuildRepresentation();
  }
  // Now render our opaque geometry:
  count += this->SurfaceActor->RenderTranslucentPolygonalGeometry(viewport);
  if (this->EdgeActor->GetVisibility())
  {
    count += this->EdgeActor->RenderTranslucentPolygonalGeometry(viewport);
  }
  return count;
}

vtkTypeBool vtkMeshInspectorRepresentation::HasTranslucentPolygonalGeometry()
{
  return this->SurfaceActor->HasTranslucentPolygonalGeometry() ||
    (this->EdgeActor->GetVisibility() && this->EdgeActor->HasTranslucentPolygonalGeometry()) ||
    (this->DrawHandles && this->Superclass::HasTranslucentPolygonalGeometry());
}

void vtkMeshInspectorRepresentation::BuildRepresentation()
{
  if (this->DrawHandles)
  {
    this->Superclass::BuildRepresentation();
  }
  if (this->ExtractNeedsUpdate)
  {
    this->UpdateBlockIds();
  }
  this->SurfaceMapper->Update();
  this->EdgeMapper->Update();
}

void vtkMeshInspectorRepresentation::UpdateBlockIds()
{
  this->ExtractNeedsUpdate = false;
  this->Extract->RemoveAllIndices();
  if (this->Input)
  {
    auto iter = vtkSmartPointer<vtkCompositeDataIterator>::Take(this->Input->NewIterator());
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
      auto uid = vtkResourceMultiBlockSource::GetDataObjectUUID(iter->GetCurrentMetaData());
      if (this->Components.find(uid) != this->Components.end())
      {
        this->Extract->AddIndex(iter->GetCurrentFlatIndex());
      }
    }
  }
}

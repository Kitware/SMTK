//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKCompositeRepresentation.h"

#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKSelectionRepresentation.h"

#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPVGridAxes3DRepresentation.h"
#include "vtkPolarAxesRepresentation.h"
#include "vtkView.h"

vtkStandardNewMacro(vtkSMTKCompositeRepresentation);
vtkCxxSetObjectMacro(
  vtkSMTKCompositeRepresentation, PolarAxesRepresentation, vtkPolarAxesRepresentation);
vtkCxxSetObjectMacro(
  vtkSMTKCompositeRepresentation, GridAxesRepresentation, vtkPVGridAxes3DRepresentation);

//----------------------------------------------------------------------------
vtkSMTKCompositeRepresentation::vtkSMTKCompositeRepresentation()
{
  this->SelectionRepresentation = vtkSMTKSelectionRepresentation::New();
  this->SelectionRepresentation->SetCompositeRepresentation(this);

  this->PolarAxesRepresentation = NULL;

  this->SelectionVisibility = false;
  this->SelectionRepresentation->SetVisibility(false);

  this->GridAxesRepresentation = vtkPVGridAxes3DRepresentation::New();
  this->GridAxesRepresentation->SetVisibility(false);
}

//----------------------------------------------------------------------------
vtkSMTKCompositeRepresentation::~vtkSMTKCompositeRepresentation()
{
  this->SetSelectionRepresentation(NULL);
  this->SetPolarAxesRepresentation(NULL);
  this->SetGridAxesRepresentation(NULL);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetSelectionRepresentation(vtkSMTKSelectionRepresentation* arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting SelectionRepresentation to "
                << arg);
  if (this->SelectionRepresentation != arg)
  {
    vtkSMTKSelectionRepresentation* tempSGMacroVar = this->SelectionRepresentation;
    this->SelectionRepresentation = arg;
    if (this->SelectionRepresentation != nullptr)
    {
      this->SelectionRepresentation->Register(this);
      this->SelectionRepresentation->SetCompositeRepresentation(this);
    }
    if (tempSGMacroVar != nullptr)
    {
      tempSGMacroVar->UnRegister(this);
    }
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetVisibility(bool visible)
{
  this->Superclass::SetVisibility(visible);
  this->SetSelectionVisibility(this->SelectionVisibility);
  this->GridAxesRepresentation->SetVisibility(visible);
  this->SetPolarAxesVisibility(visible);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetSelectionVisibility(bool visible)
{
  this->SelectionVisibility = visible;
  this->SelectionRepresentation->SetVisibility(this->GetVisibility() && visible);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetPolarAxesVisibility(bool visible)
{
  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->SetParentVisibility(visible);
  }
}

//----------------------------------------------------------------------------
bool vtkSMTKCompositeRepresentation::AddToView(vtkView* view)
{
  if (!this->Superclass::AddToView(view))
  {
    return false;
  }

  view->AddRepresentation(this->SelectionRepresentation);
  view->AddRepresentation(this->GridAxesRepresentation);

  if (this->PolarAxesRepresentation)
  {
    view->AddRepresentation(this->PolarAxesRepresentation);
  }

  // a good spot to update debug names for internal representations.
  this->SetDebugName(this->SelectionRepresentation, "Selection");
  this->SetDebugName(this->PolarAxesRepresentation, "PolarAxes");
  this->SetDebugName(this->GridAxesRepresentation, "GridAxes");
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMTKCompositeRepresentation::RemoveFromView(vtkView* view)
{
  view->RemoveRepresentation(this->SelectionRepresentation);
  view->RemoveRepresentation(this->GridAxesRepresentation);

  if (this->PolarAxesRepresentation)
  {
    view->RemoveRepresentation(this->PolarAxesRepresentation);
  }

  return this->Superclass::RemoveFromView(view);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::MarkModified()
{
  this->SelectionRepresentation->MarkModified();
  this->GridAxesRepresentation->MarkModified();

  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->MarkModified();
  }

  this->Superclass::MarkModified();
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetUpdateTime(double time)
{
  this->SelectionRepresentation->SetUpdateTime(time);
  this->GridAxesRepresentation->SetUpdateTime(time);

  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->SetUpdateTime(time);
  }

  this->Superclass::SetUpdateTime(time);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetForceUseCache(bool val)
{
  this->SelectionRepresentation->SetForceUseCache(val);
  this->GridAxesRepresentation->SetForceUseCache(val);

  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->SetForceUseCache(val);
  }

  this->Superclass::SetForceUseCache(val);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetForcedCacheKey(double val)
{
  this->SelectionRepresentation->SetForcedCacheKey(val);
  this->GridAxesRepresentation->SetForcedCacheKey(val);

  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->SetForcedCacheKey(val);
  }

  this->Superclass::SetForcedCacheKey(val);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetInputConnection(int port, vtkAlgorithmOutput* input)
{
  this->GridAxesRepresentation->SetInputConnection(port, input);
  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->SetInputConnection(port, input);
  }
  this->Superclass::SetInputConnection(port, input);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetInputConnection(vtkAlgorithmOutput* input)
{
  this->GridAxesRepresentation->SetInputConnection(input);
  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->SetInputConnection(input);
  }
  this->Superclass::SetInputConnection(input);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::AddInputConnection(int port, vtkAlgorithmOutput* input)
{
  this->GridAxesRepresentation->AddInputConnection(port, input);
  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->AddInputConnection(port, input);
  }
  this->Superclass::AddInputConnection(port, input);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::AddInputConnection(vtkAlgorithmOutput* input)
{
  this->GridAxesRepresentation->AddInputConnection(input);
  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->AddInputConnection(input);
  }
  this->Superclass::AddInputConnection(input);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::RemoveInputConnection(int port, vtkAlgorithmOutput* input)
{
  this->GridAxesRepresentation->RemoveInputConnection(port, input);
  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->RemoveInputConnection(port, input);
  }
  this->Superclass::RemoveInputConnection(port, input);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::RemoveInputConnection(int port, int idx)
{
  this->GridAxesRepresentation->RemoveInputConnection(port, idx);
  if (this->PolarAxesRepresentation)
  {
    this->PolarAxesRepresentation->RemoveInputConnection(port, idx);
  }
  this->Superclass::RemoveInputConnection(port, idx);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetPointFieldDataArrayName(const char* val)
{
  // this->SelectionRepresentation->SetPointFieldDataArrayName(val);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::SetCellFieldDataArrayName(const char* val)
{
  // this->SelectionRepresentation->SetCellFieldDataArrayName(val);
}

//----------------------------------------------------------------------------
void vtkSMTKCompositeRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
//----------------------------------------------------------------------------
unsigned int vtkSMTKCompositeRepresentation::Initialize(
  unsigned int minIdAvailable, unsigned int maxIdAvailable)
{
  unsigned int minId = minIdAvailable;
  minId = this->SelectionRepresentation->Initialize(minId, maxIdAvailable);
  minId = this->GridAxesRepresentation->Initialize(minId, maxIdAvailable);
  if (this->PolarAxesRepresentation)
  {
    minId = this->PolarAxesRepresentation->Initialize(minId, maxIdAvailable);
  }
  return this->Superclass::Initialize(minId, maxIdAvailable);
}

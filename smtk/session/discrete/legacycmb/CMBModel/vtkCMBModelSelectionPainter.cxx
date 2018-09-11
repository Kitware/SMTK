//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelSelectionPainter.h"

#include "vtkCompositeDataIterator.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkGarbageCollector.h"
#include "vtkHardwareSelector.h"
#include "vtkModelEdge.h"
#include "vtkModelFace.h"
#include "vtkModelGeometricEntity.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include <map>
#ifndef VTK_IMPLEMENT_MESA_CXX
#include "vtkOpenGL.h"
#endif
vtkStandardNewMacro(vtkCMBModelSelectionPainter);
vtkCMBModelSelectionPainter::vtkCMBModelSelectionPainter()
{
  this->OutputData = 0;
}

vtkCMBModelSelectionPainter::~vtkCMBModelSelectionPainter()
{
}

vtkDataObject* vtkCMBModelSelectionPainter::GetOutput()
{
  return this->OutputData ? this->OutputData : this->GetInput();
}

void vtkCMBModelSelectionPainter::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);

  vtkGarbageCollectorReport(collector, this->OutputData, "Output");
}

void inline RenderEntities(std::map<unsigned int, vtkDataObject*>& entList,
  vtkHardwareSelector* selector, vtkInformation* thisInformation, vtkPainter* delegatePainter,
  vtkDataObject* outputData, vtkRenderer* renderer, vtkActor* actor, unsigned long typeflags,
  bool forceCompileOnly)
{
  std::map<unsigned int, vtkDataObject*>::iterator renderIt;
  for (renderIt = entList.begin(); renderIt != entList.end(); ++renderIt)
  {
    unsigned int flat_index = (*renderIt).first;
    vtkDataObject* dobj = (*renderIt).second;
    selector->BeginRenderProp();
    // If hardware selection is in progress, we need to pass the composite
    // index to the selection framework,
    selector->RenderCompositeIndex(flat_index);

    delegatePainter->SetInput(dobj);
    outputData = dobj;
    if (thisInformation != delegatePainter->GetInformation())
    {
      // We have updated information, pass it on to
      // the delegate.
      delegatePainter->SetInformation(thisInformation);
    }

    //this->UpdateDelegatePainter();
    delegatePainter->Render(renderer, actor, typeflags, forceCompileOnly);
    outputData = 0;

    selector->EndRenderProp();
  }
}

void vtkCMBModelSelectionPainter::RenderInternal(
  vtkRenderer* renderer, vtkActor* actor, unsigned long typeflags, bool forceCompileOnly)
{
  vtkDiscreteModelWrapper* inputModel = vtkDiscreteModelWrapper::SafeDownCast(this->GetInput());
  if (!inputModel || !this->DelegatePainter)
  {
    this->vtkPainter::RenderInternal(renderer, actor, typeflags, forceCompileOnly);
    return;
  }
  vtkHardwareSelector* selector = renderer->GetSelector();
  if (!selector)
  {
    return;
  }
  unsigned int curr_idx;
  vtkModelGeometricEntity* entity;

  // For selection to work with the cmb model, the rendering order has to be
  // Vertex, Line, Faces; so first loop is to sort out the ordered list, then
  // render them for selection
  std::map<unsigned int, vtkDataObject *> vertsList, edgesList, facesList;
  vtkCompositeDataIterator* iter = inputModel->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkDataObject* dobj = iter->GetCurrentDataObject();
    if (!dobj)
    {
      continue;
    }
    curr_idx = iter->GetCurrentFlatIndex();
    entity =
      vtkModelGeometricEntity::SafeDownCast(inputModel->GetEntityObjectByFlatIndex(curr_idx));
    if (!entity || !entity->GetVisibility() || !entity->GetPickable())
    {
      continue;
    }
    if (entity->IsA("vtkModelVertex"))
    {
      vertsList[curr_idx] = dobj;
    }
    else if (entity->IsA("vtkModelEdge"))
    {
      edgesList[curr_idx] = dobj;
    }
    else if (entity->IsA("vtkModelFace"))
    {
      facesList[curr_idx] = dobj;
    }
  }
  iter->Delete();
  RenderEntities(facesList, selector, this->Information, this->DelegatePainter, this->OutputData,
    renderer, actor, typeflags, forceCompileOnly);
  RenderEntities(edgesList, selector, this->Information, this->DelegatePainter, this->OutputData,
    renderer, actor, typeflags, forceCompileOnly);
  RenderEntities(vertsList, selector, this->Information, this->DelegatePainter, this->OutputData,
    renderer, actor, typeflags, forceCompileOnly);
}

void vtkCMBModelSelectionPainter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

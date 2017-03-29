//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelMapper.h"

#include "vtkBoundingBox.h"
#include "vtkCMBModelSelectionPainter.h"
#include "vtkChooserPainter.h"
#include "vtkCommand.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkCompositeDataSet.h"
#include "vtkDisplayListPainter.h"
#include "vtkHardwareSelector.h"
#include "vtkInformation.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGL.h"
#include "vtkPolyData.h"
#include "vtkPolyDataPainter.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkTexture.h"
#include "vtkTimerLog.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelGeometricEntity.h"

vtkStandardNewMacro(vtkCMBModelMapper);
vtkCxxSetObjectMacro(vtkCMBModelMapper, CMBModel, vtkDiscreteModelWrapper);

//----------------------------------------------------------------------------
vtkCMBModelMapper::vtkCMBModelMapper()
{
  // Insert the vtkCMBModelSelectionPainter in the selection pipeline, so that the
  // selection painter can handle cmb model datasets as well.
  vtkCMBModelSelectionPainter* modelPainter = vtkCMBModelSelectionPainter::New();
  if(this->SelectionPainter)
    {
    modelPainter->SetDelegatePainter(
      this->SelectionPainter->GetDelegatePainter() ?
      this->SelectionPainter->GetDelegatePainter() :
      this->SelectionPainter);
    }
  this->SetSelectionPainter(modelPainter);
  modelPainter->FastDelete();
  this->CMBModel = NULL;
  this->SetNumberOfOutputPorts(1);
  this->ShowEdgePoints = false;
  this->ModelDisplayListId = 0;
}

//----------------------------------------------------------------------------
vtkCMBModelMapper::~vtkCMBModelMapper()
{
if (this->LastWindow)
  {
  this->ReleaseGraphicsResources(this->LastWindow);
  }
  this->SetCMBModel(0);
  this->SetSelectionPainter(0);
}

//----------------------------------------------------------------------------
int vtkCMBModelMapper::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCMBModelMapper");
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkCMBModelMapper::FillOutputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDiscreteModelWrapper");
  return 1;
}

// Release the graphics resources used by this mapper.  In this case, release
// the display list if any.
//-----------------------------------------------------------------------------
void vtkCMBModelMapper::ReleaseGraphicsResources(vtkWindow *win)
{
  if (this->ModelDisplayListId && win && win->GetMapped())
    {
    win->MakeCurrent();
    glDeleteLists(this->ModelDisplayListId,1);
    }
  this->ModelDisplayListId = 0;
  this->Superclass::ReleaseGraphicsResources(win);
}

//-----------------------------------------------------------------------------
void vtkCMBModelMapper::RenderPiece(vtkRenderer* ren, vtkActor* act)
{
  vtkDataObject *input= this->GetInputDataObject(0, 0);
  //
  // make sure that we've been properly initialized
  //
  if (ren->GetRenderWindow()->CheckAbortStatus())
    {
    return;
    }

  if ( input == NULL )
    {
    vtkErrorMacro(<< "No input!");
    return;
    }
  else
    {
    this->InvokeEvent(vtkCommand::StartEvent,NULL);
    if (!this->Static)
      {
      this->GetInputAlgorithm()->Update();
      }
    this->InvokeEvent(vtkCommand::EndEvent,NULL);
    }
  if ( this->CMBModel == NULL )
    {
    vtkErrorMacro(<< "No cmb model!");
    return;
    }

  // Update Painter information if obsolete.
  if (this->PainterUpdateTime < this->GetMTime())
    {
    // turn on Immediate rendering so that the down stream
    // painters do not use their own display list. This mappping is handling
    // its own display list.
    this->ImmediateModeRendering = 1;
    this->UpdatePainterInformation();
    this->PainterUpdateTime.Modified();
    }

  // make sure our window is current
  ren->GetRenderWindow()->MakeCurrent();
  this->TimeToDraw = 0.0;

  // If we are rendering in selection mode, then we use the selection painter
  // instead of the standard painter.
  if (this->SelectionPainter && ren->GetSelector())
    {
    this->SelectionPainter->SetInput(this->CMBModel);
    this->SelectionPainter->Render(ren, act, 0xff,
      (this->ForceCompileOnly==1));
    this->TimeToDraw = this->SelectionPainter->GetTimeToDraw();
    }
  else if (this->SelectionPainter && this->SelectionPainter != this->Painter)
    {
    this->SelectionPainter->ReleaseGraphicsResources(ren->GetRenderWindow());
    }

  if (this->Painter && ren->GetSelector() == 0)
    {
    this->RenderInternal(input, ren, act,
      0xff,this->ForceCompileOnly==1);
    //this->TimeToDraw = this->Painter->GetTimeToDraw();
    this->TimeToDraw = this->Timer->GetElapsedTime();
    }

  // If the timer is not accurate enough, set it to a small
  // time so that it is not zero
  if ( this->TimeToDraw == 0.0 )
    {
    this->TimeToDraw = 0.0001;
    }

  this->UpdateProgress(1.0);
}

//----------------------------------------------------------------------------
void vtkCMBModelMapper::RenderInternal( vtkDataObject* inputObj,
  vtkRenderer* renderer, vtkActor* actor,
  unsigned long typeflags, bool forceCompileOnly)
{
  vtkCompositeDataSet* input = vtkCompositeDataSet::SafeDownCast(
    inputObj);
  if (!input)
    {
    vtkErrorMacro(<< "Expecting composite data input of CMB model!");
    return;
    }
  vtkDiscreteModelWrapper *model= this->CMBModel;
  if (!model)
    {
    return;
    }
  vtkProperty* actorProperty = actor->GetProperty();
  vtkProperty* backFaceProperty = actor->GetBackfaceProperty();
  // Time the actual drawing
  this->Timer->StartTimer();

  if ( this->GetMTime() > this->BuildTime ||
    input->GetMTime() > this->BuildTime ||
    actorProperty->GetMTime() > this->BuildTime ||
    renderer->GetRenderWindow() != this->LastWindow)
    {
    this->ReleaseGraphicsResources(renderer->GetRenderWindow());

    // get a unique display list id
     this->ModelDisplayListId = glGenLists(1);
     glNewList(this->ModelDisplayListId, GL_COMPILE);

    //actorProperty->SetOpacity(1.0);
    // cache what's original in model actor, since
    // these property may be changed depending on model entity types
    double actorOpacity = actorProperty->GetOpacity();
    int actorRep = actorProperty->GetRepresentation();
    int actorEdgeVis = actorProperty->GetEdgeVisibility();
    double actorPtSzie = actorProperty->GetPointSize();
    double actorLineWidth = actorProperty->GetLineWidth();
    vtkTexture* texture = actor->GetTexture();

    unsigned int curr_idx;
    vtkProperty* pProp;
    vtkModelGeometricEntity* entity;
    vtkDiscreteModelEdge* edgeEntity;
    vtkDataObject* dobj;
    int isFaceType;
    vtkCompositeDataIterator* iter = input->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
      {
      curr_idx = iter->GetCurrentFlatIndex();
      entity = vtkModelGeometricEntity::SafeDownCast(
        model->GetEntityObjectByFlatIndex(curr_idx));
      if(!entity || !entity->GetVisibility())
        {
        continue;
        }

      pProp = entity->GetDisplayProperty();
      if(!pProp)
        {
        continue;
        }

      // we always use color from model entity
      actorProperty->SetColor(pProp->GetColor());

      isFaceType  = entity->IsA("vtkDiscreteModelFace");
      actorProperty->SetRepresentation(isFaceType ? actorRep :
        pProp->GetRepresentation());
      actorProperty->SetEdgeVisibility(isFaceType ? actorEdgeVis :
        pProp->GetEdgeVisibility());
      actorProperty->SetPointSize(isFaceType ? actorPtSzie :
        pProp->GetPointSize());
      actorProperty->SetOpacity(isFaceType ? actorOpacity :
        pProp->GetOpacity());
      actorProperty->SetLineWidth(isFaceType ? actorLineWidth :
        pProp->GetLineWidth());

      actorProperty->Render(actor, renderer);

      if(backFaceProperty)
        {
        backFaceProperty->Render(actor, renderer);
        }
      if(texture && entity->GetShowTexture())
        {
        texture->Render(renderer);
        }

      if(entity->IsA("vtkDiscreteModelEdge") && this->ShowEdgePoints)
        {
        edgeEntity = vtkDiscreteModelEdge::SafeDownCast(entity);
        dobj = vtkDataObject::SafeDownCast(
          edgeEntity->GetLineAndPointsGeometry());
        }
      else
        {
        dobj = iter->GetCurrentDataObject();
        }

      this->Painter->SetInput(dobj);
      this->Painter->Render(renderer, actor, typeflags, forceCompileOnly);
      }
    iter->Delete();

    glEndList();
    this->BuildTime.Modified();
    this->LastWindow = renderer->GetRenderWindow();
    glCallList(this->ModelDisplayListId);

    }
  else
    {
    // Pass input.
    glCallList(this->ModelDisplayListId);
    }

  this->Timer->StopTimer();
}

//-----------------------------------------------------------------------------
void vtkCMBModelMapper::ComputeBounds()
{
  if(this->BuildTime < this->BoundsMTime &&
    this->GetMTime() < this->BoundsMTime &&
    this->CMBModel->GetMTime() < this->BoundsMTime )
    {
    return;
    }
  vtkMath::UninitializeBounds(this->Bounds);
  vtkCompositeDataSet *input = vtkCompositeDataSet::SafeDownCast(
    this->GetInputDataObject(0, 0));

  // If we don't have hierarchical data, test to see if we have
  // plain old polydata. In this case, the bounds are simply
  // the bounds of the input polydata.
  if (!input)
    {
    this->Superclass::ComputeBounds();
    return;
    }

  vtkBoundingBox bbox;
  unsigned int curr_idx;
  vtkModelGeometricEntity* entity;
  vtkCompositeDataIterator* iter = input->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    curr_idx = iter->GetCurrentFlatIndex();
    entity = vtkModelGeometricEntity::SafeDownCast(
      this->CMBModel->GetEntityObjectByFlatIndex(curr_idx));
    if(!entity || !entity->GetVisibility())
      {
      continue;
      }
    vtkPolyData *pd = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
    if (pd)
      {
      double bounds[6];
      pd->GetBounds(bounds);
      bbox.AddBounds(bounds);
      }
    }
  iter->Delete();
  bbox.GetBounds(this->Bounds);
  this->BoundsMTime.Modified();
}

//----------------------------------------------------------------------------
void vtkCMBModelMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

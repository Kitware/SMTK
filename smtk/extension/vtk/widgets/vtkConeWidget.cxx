//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/widgets/vtkConeWidget.h"
#include "smtk/extension/vtk/widgets/vtkConeRepresentation.h"

#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkEvent.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkStdString.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"

vtkStandardNewMacro(vtkConeWidget);

//----------------------------------------------------------------------------
vtkConeWidget::vtkConeWidget()
{
  this->WidgetState = vtkConeWidget::Start;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent, vtkWidgetEvent::Select, this, vtkConeWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkWidgetEvent::EndSelect,
    this,
    vtkConeWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MiddleButtonPressEvent,
    vtkWidgetEvent::Translate,
    this,
    vtkConeWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MiddleButtonReleaseEvent,
    vtkWidgetEvent::EndTranslate,
    this,
    vtkConeWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::RightButtonPressEvent, vtkWidgetEvent::Scale, this, vtkConeWidget::ScaleAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::RightButtonReleaseEvent,
    vtkWidgetEvent::EndScale,
    this,
    vtkConeWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, vtkConeWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    30,
    1,
    "Up",
    vtkWidgetEvent::Up,
    this,
    vtkConeWidget::MoveConeAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    28,
    1,
    "Right",
    vtkWidgetEvent::Up,
    this,
    vtkConeWidget::MoveConeAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    31,
    1,
    "Down",
    vtkWidgetEvent::Down,
    this,
    vtkConeWidget::MoveConeAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    29,
    1,
    "Left",
    vtkWidgetEvent::Down,
    this,
    vtkConeWidget::MoveConeAction);
}

//----------------------------------------------------------------------------
vtkConeWidget::~vtkConeWidget() = default;

//----------------------------------------------------------------------
void vtkConeWidget::SelectAction(vtkAbstractWidget* w)
{
  vtkConeWidget* self = reinterpret_cast<vtkConeWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // We want to update the radius, axis and center as appropriate
  reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkConeRepresentation::Moving);
  int interactionState = self->WidgetRep->ComputeInteractionState(X, Y);
  self->UpdateCursorShape(interactionState);

  if (self->WidgetRep->GetInteractionState() == vtkConeRepresentation::Outside)
  {
    return;
  }

  if (self->Interactor->GetControlKey() && interactionState == vtkConeRepresentation::MovingWhole)
  {
    reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)
      ->SetInteractionState(vtkConeRepresentation::TranslatingCenter);
  }

  // We are definitely selected
  self->GrabFocus(self->EventCallbackCommand);
  double eventPos[2];
  eventPos[0] = static_cast<double>(X);
  eventPos[1] = static_cast<double>(Y);
  self->WidgetState = vtkConeWidget::Active;
  self->WidgetRep->StartWidgetInteraction(eventPos);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkConeWidget::TranslateAction(vtkAbstractWidget* w)
{
  vtkConeWidget* self = reinterpret_cast<vtkConeWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // We want to compute an orthogonal vector to the pane that has been selected
  reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkConeRepresentation::Moving);
  int interactionState = self->WidgetRep->ComputeInteractionState(X, Y);
  self->UpdateCursorShape(interactionState);

  if (self->WidgetRep->GetInteractionState() == vtkConeRepresentation::Outside)
  {
    return;
  }

  // We are definitely selected
  self->GrabFocus(self->EventCallbackCommand);
  double eventPos[2];
  eventPos[0] = static_cast<double>(X);
  eventPos[1] = static_cast<double>(Y);
  self->WidgetState = vtkConeWidget::Active;
  self->WidgetRep->StartWidgetInteraction(eventPos);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkConeWidget::ScaleAction(vtkAbstractWidget* w)
{
  vtkConeWidget* self = reinterpret_cast<vtkConeWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // We want to compute an orthogonal vector to the pane that has been selected
  reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkConeRepresentation::Scaling);
  int interactionState = self->WidgetRep->ComputeInteractionState(X, Y);
  self->UpdateCursorShape(interactionState);

  if (self->WidgetRep->GetInteractionState() == vtkConeRepresentation::Outside)
  {
    return;
  }

  // We are definitely selected
  self->GrabFocus(self->EventCallbackCommand);
  double eventPos[2];
  eventPos[0] = static_cast<double>(X);
  eventPos[1] = static_cast<double>(Y);
  self->WidgetState = vtkConeWidget::Active;
  self->WidgetRep->StartWidgetInteraction(eventPos);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkConeWidget::MoveAction(vtkAbstractWidget* w)
{
  vtkConeWidget* self = reinterpret_cast<vtkConeWidget*>(w);

  // So as to change the cursor shape when the mouse is poised over
  // the widget. Unfortunately, this results in a few extra picks
  // due to the cell picker. However given that its picking simple geometry
  // like the handles/arrows, this should be very quick
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  int changed = 0;

  if (self->ManagesCursor && self->WidgetState != vtkConeWidget::Active)
  {
    int oldInteractionState =
      reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)->GetInteractionState();

    reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)
      ->SetInteractionState(vtkConeRepresentation::Moving);
    int state = self->WidgetRep->ComputeInteractionState(X, Y);
    changed = self->UpdateCursorShape(state);
    reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)
      ->SetInteractionState(oldInteractionState);
    changed = (changed || state != oldInteractionState) ? 1 : 0;
  }

  // See whether we're active
  if (self->WidgetState == vtkConeWidget::Start)
  {
    if (changed && self->ManagesCursor)
    {
      self->Render();
    }
    return;
  }

  // Okay, adjust the representation
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->WidgetInteraction(e);

  // moving something
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkConeWidget::EndSelectAction(vtkAbstractWidget* w)
{
  vtkConeWidget* self = reinterpret_cast<vtkConeWidget*>(w);

  if (
    self->WidgetState != vtkConeWidget::Active ||
    self->WidgetRep->GetInteractionState() == vtkConeRepresentation::Outside)
  {
    return;
  }

  // Return state to not selected
  double e[2];
  self->WidgetRep->EndWidgetInteraction(e);
  self->WidgetState = vtkConeWidget::Start;
  self->ReleaseFocus();

  // Update cursor if managed
  self->UpdateCursorShape(
    reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)->GetRepresentationState());

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkConeWidget::MoveConeAction(vtkAbstractWidget* w)
{
  vtkConeWidget* self = reinterpret_cast<vtkConeWidget*>(w);

  reinterpret_cast<vtkConeRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkConeRepresentation::Moving);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  self->WidgetRep->ComputeInteractionState(X, Y);

  // The cursor must be over part of the widget for these key presses to work
  if (self->WidgetRep->GetInteractionState() == vtkConeRepresentation::Outside)
  {
    return;
  }

  // Invoke all of the events associated with moving the cylinder
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);

  // Move the cylinder
  double factor = (self->Interactor->GetControlKey() ? 0.5 : 1.0);
  if (
    vtkStdString(self->Interactor->GetKeySym()) == vtkStdString("Down") ||
    vtkStdString(self->Interactor->GetKeySym()) == vtkStdString("Left"))
  {
    self->GetConeRepresentation()->BumpCone(-1, factor);
  }
  else
  {
    self->GetConeRepresentation()->BumpCone(1, factor);
  }
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkConeWidget::SetEnabled(int enabling)
{
  if (this->Enabled == enabling)
  {
    return;
  }

  Superclass::SetEnabled(enabling);
}

//----------------------------------------------------------------------
void vtkConeWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkConeRepresentation::New();
  }
}

//----------------------------------------------------------------------
void vtkConeWidget::SetRepresentation(vtkConeRepresentation* rep)
{
  this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(rep));
}

//----------------------------------------------------------------------
int vtkConeWidget::UpdateCursorShape(int state)
{
  // So as to change the cursor shape when the mouse is poised over
  // the widget.
  if (this->ManagesCursor)
  {
    if (state == vtkConeRepresentation::Outside)
    {
      return this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    }
    else if (state == vtkConeRepresentation::AdjustingBottomRadius)
    {
      return this->RequestCursorShape(VTK_CURSOR_SIZEALL);
    }
    else
    {
      return this->RequestCursorShape(VTK_CURSOR_HAND);
    }
  }

  return 0;
}

//----------------------------------------------------------------------------
void vtkConeWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

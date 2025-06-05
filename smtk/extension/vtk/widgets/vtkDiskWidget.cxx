//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/widgets/vtkDiskWidget.h"
#include "smtk/extension/vtk/widgets/vtkDiskRepresentation.h"

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

vtkStandardNewMacro(vtkDiskWidget);

//----------------------------------------------------------------------------
vtkDiskWidget::vtkDiskWidget()
{
  this->WidgetState = vtkDiskWidget::Start;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent, vtkWidgetEvent::Select, this, vtkDiskWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkWidgetEvent::EndSelect,
    this,
    vtkDiskWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MiddleButtonPressEvent,
    vtkWidgetEvent::Translate,
    this,
    vtkDiskWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MiddleButtonReleaseEvent,
    vtkWidgetEvent::EndTranslate,
    this,
    vtkDiskWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::RightButtonPressEvent, vtkWidgetEvent::Scale, this, vtkDiskWidget::ScaleAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::RightButtonReleaseEvent,
    vtkWidgetEvent::EndScale,
    this,
    vtkDiskWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, vtkDiskWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    30,
    1,
    "Up",
    vtkWidgetEvent::Up,
    this,
    vtkDiskWidget::MoveDiskAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    28,
    1,
    "Right",
    vtkWidgetEvent::Up,
    this,
    vtkDiskWidget::MoveDiskAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    31,
    1,
    "Down",
    vtkWidgetEvent::Down,
    this,
    vtkDiskWidget::MoveDiskAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    29,
    1,
    "Left",
    vtkWidgetEvent::Down,
    this,
    vtkDiskWidget::MoveDiskAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    'n',
    1,
    "n",
    vtkWidgetEvent::PickNormal,
    this,
    vtkDiskWidget::PickNormalAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    'N',
    1,
    "N",
    vtkWidgetEvent::PickNormal,
    this,
    vtkDiskWidget::PickNormalAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::KeyPressEvent,
    vtkEvent::AnyModifier,
    14,
    1,
    "n",
    vtkWidgetEvent::PickNormal,
    this,
    vtkDiskWidget::PickNormalAction);
}

//----------------------------------------------------------------------------
vtkDiskWidget::~vtkDiskWidget() = default;

//----------------------------------------------------------------------
void vtkDiskWidget::SelectAction(vtkAbstractWidget* w)
{
  vtkDiskWidget* self = reinterpret_cast<vtkDiskWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // We want to update the radius, normal, and center as appropriate
  reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkDiskRepresentation::Moving);
  int interactionState = self->WidgetRep->ComputeInteractionState(X, Y);
  self->UpdateCursorShape(interactionState);

  if (self->WidgetRep->GetInteractionState() == vtkDiskRepresentation::Outside)
  {
    return;
  }

  if (self->Interactor->GetControlKey() && interactionState == vtkDiskRepresentation::MovingWhole)
  {
    reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)
      ->SetInteractionState(vtkDiskRepresentation::MovingWhole);
  }

  // We are definitely selected
  self->GrabFocus(self->EventCallbackCommand);
  double eventPos[2];
  eventPos[0] = static_cast<double>(X);
  eventPos[1] = static_cast<double>(Y);
  self->WidgetState = vtkDiskWidget::Active;
  self->WidgetRep->StartWidgetInteraction(eventPos);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkDiskWidget::TranslateAction(vtkAbstractWidget* w)
{
  vtkDiskWidget* self = reinterpret_cast<vtkDiskWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // We want to compute an orthogonal vector to the pane that has been selected
  reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkDiskRepresentation::Moving);
  int interactionState = self->WidgetRep->ComputeInteractionState(X, Y);
  self->UpdateCursorShape(interactionState);

  if (self->WidgetRep->GetInteractionState() == vtkDiskRepresentation::Outside)
  {
    return;
  }

  // We are definitely selected
  self->GrabFocus(self->EventCallbackCommand);
  double eventPos[2];
  eventPos[0] = static_cast<double>(X);
  eventPos[1] = static_cast<double>(Y);
  self->WidgetState = vtkDiskWidget::Active;
  self->WidgetRep->StartWidgetInteraction(eventPos);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkDiskWidget::ScaleAction(vtkAbstractWidget* w)
{
  vtkDiskWidget* self = reinterpret_cast<vtkDiskWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // We want to compute an orthogonal vector to the pane that has been selected
  reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkDiskRepresentation::AdjustingRadius);
  int interactionState = self->WidgetRep->ComputeInteractionState(X, Y);
  self->UpdateCursorShape(interactionState);

  if (self->WidgetRep->GetInteractionState() == vtkDiskRepresentation::Outside)
  {
    return;
  }

  // We are definitely selected
  self->GrabFocus(self->EventCallbackCommand);
  double eventPos[2];
  eventPos[0] = static_cast<double>(X);
  eventPos[1] = static_cast<double>(Y);
  self->WidgetState = vtkDiskWidget::Active;
  self->WidgetRep->StartWidgetInteraction(eventPos);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkDiskWidget::MoveAction(vtkAbstractWidget* w)
{
  vtkDiskWidget* self = reinterpret_cast<vtkDiskWidget*>(w);

  // So as to change the cursor shape when the mouse is poised over
  // the widget. Unfortunately, this results in a few extra picks
  // due to the cell picker. However given that its picking simple geometry
  // like the handles/arrows, this should be very quick
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  int changed = 0;

  if (self->ManagesCursor && self->WidgetState != vtkDiskWidget::Active)
  {
    int oldInteractionState =
      reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)->GetInteractionState();

    reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)
      ->SetInteractionState(vtkDiskRepresentation::Moving);
    int state = self->WidgetRep->ComputeInteractionState(X, Y);
    changed = self->UpdateCursorShape(state);
    reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)
      ->SetInteractionState(oldInteractionState);
    changed = (changed || state != oldInteractionState) ? 1 : 0;
  }

  // See whether we're active
  if (self->WidgetState == vtkDiskWidget::Start)
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
void vtkDiskWidget::EndSelectAction(vtkAbstractWidget* w)
{
  vtkDiskWidget* self = reinterpret_cast<vtkDiskWidget*>(w);

  if (
    self->WidgetState != vtkDiskWidget::Active ||
    self->WidgetRep->GetInteractionState() == vtkDiskRepresentation::Outside)
  {
    return;
  }

  // Return state to not selected
  double e[2];
  self->WidgetRep->EndWidgetInteraction(e);
  self->WidgetState = vtkDiskWidget::Start;
  self->ReleaseFocus();

  // Update cursor if managed
  self->UpdateCursorShape(
    reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)->GetRepresentationState());

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkDiskWidget::MoveDiskAction(vtkAbstractWidget* w)
{
  vtkDiskWidget* self = reinterpret_cast<vtkDiskWidget*>(w);

  reinterpret_cast<vtkDiskRepresentation*>(self->WidgetRep)
    ->SetInteractionState(vtkDiskRepresentation::Moving);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  self->WidgetRep->ComputeInteractionState(X, Y);

  // The cursor must be over part of the widget for these key presses to work
  if (self->WidgetRep->GetInteractionState() == vtkDiskRepresentation::Outside)
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
    self->GetDiskRepresentation()->BumpDisk(-1, factor);
  }
  else
  {
    self->GetDiskRepresentation()->BumpDisk(1, factor);
  }
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkDiskWidget::PickNormalAction(vtkAbstractWidget* w)
{
  vtkDiskWidget* self = reinterpret_cast<vtkDiskWidget*>(w);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Invoke all the events associated with moving the plane
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  bool newNormalPicked =
    self->GetDiskRepresentation()->PickNormal(X, Y, self->Interactor->GetControlKey() == 1);
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  if (newNormalPicked)
  {
    self->Render();
  }
}

//----------------------------------------------------------------------
void vtkDiskWidget::SetEnabled(int enabling)
{
  if (this->Enabled == enabling)
  {
    return;
  }

  Superclass::SetEnabled(enabling);
}

//----------------------------------------------------------------------
void vtkDiskWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkDiskRepresentation::New();
  }
}

//----------------------------------------------------------------------
void vtkDiskWidget::SetRepresentation(vtkDiskRepresentation* rep)
{
  this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(rep));
}

//----------------------------------------------------------------------
int vtkDiskWidget::UpdateCursorShape(int state)
{
  // So as to change the cursor shape when the mouse is poised over
  // the widget.
  if (this->ManagesCursor)
  {
    if (state == vtkDiskRepresentation::Outside)
    {
      return this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    }
    else if (state == vtkDiskRepresentation::AdjustingRadius)
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
void vtkDiskWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

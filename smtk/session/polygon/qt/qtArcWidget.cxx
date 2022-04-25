//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/polygon/qt/qtArcWidget.h"
#include "ui_qtArcWidget.h"

#include <QDoubleValidator>
#include <QMessageBox>
#include <QShortcut>
#include <QtDebug>

#include "vtkAbstractWidget.h"
#include "vtkCommand.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"

class qtArcWidget::qtInternals : public Ui::qtArcWidget
{
public:
  vtkNew<vtkEventQtSlotConnect> ClosedLoopConnect;
};

qtArcWidget::qtArcWidget(QWidget* parentWdg)
  : Superclass(
      qtInteractionWidget::createWidget("representations", "smtkArcWidgetRepresentation"),
      parentWdg)
  , Internals(new qtArcWidget::qtInternals())
{
  this->Internals->setupUi(this);

  this->Internals->Visibility->setChecked(this->isInteractivityEnabled());
  this->connect(
    this->Internals->Visibility, SIGNAL(toggled(bool)), SLOT(setEnableInteractivity(bool)));
  this->Internals->Visibility->connect(
    this, SIGNAL(enableInteractivityChanged(bool)), SLOT(setChecked(bool)));

  QObject::connect(this->Internals->Closed, SIGNAL(toggled(bool)), this, SLOT(closeLoop(bool)));

  QObject::connect(this->Internals->Delete, SIGNAL(clicked()), this, SLOT(deleteAllNodes()));

  QObject::connect(this->Internals->EditMode, SIGNAL(toggled(bool)), this, SLOT(updateMode()));
  QObject::connect(this->Internals->ModifyMode, SIGNAL(toggled(bool)), this, SLOT(updateMode()));
  QObject::connect(this->Internals->Finished, SIGNAL(clicked()), this, SLOT(finishContour()));
  QPushButton* finishButton = this->Internals->Finished;
  QPalette applyPalette = finishButton->palette();
  applyPalette.setColor(QPalette::Active, QPalette::Button, QColor(161, 213, 135));
  applyPalette.setColor(QPalette::Inactive, QPalette::Button, QColor(161, 213, 135));
  finishButton->setPalette(applyPalette);
  finishButton->setDefault(true);

  this->Internals->ClosedLoopConnect->Connect(
    this->widgetProxy(), vtkCommand::EndInteractionEvent, this, SLOT(checkContourLoopClosed()));
}

qtArcWidget::~qtArcWidget() = default;

void qtArcWidget::enableApplyButton(bool state)
{
  this->Internals->Finished->setEnabled(state);
}

void qtArcWidget::deleteAllNodes()
{
  QMessageBox msgBox;
  msgBox.setText("Delete all contour nodes.");
  msgBox.setInformativeText("Do you want to delete everything you have drawn?");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Ok)
  {
    this->removeAllNodes();
  }
}

void qtArcWidget::removeAllNodes()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->widgetProxy();
  if (widget)
  {
    widget->InvokeCommand("ClearAllNodes");
    this->render();
  }
}

void qtArcWidget::checkContourLoopClosed()
{
  vtkSMProxy* repProxy = this->widgetProxy()->GetRepresentationProxy();

  // request from the info the state of the loop not on the property it self
  vtkSMPropertyHelper loopHelper(repProxy, "ClosedLoopInfo");
  loopHelper.UpdateValueFromServer();

  int loopClosed = loopHelper.GetAsInt();
  this->Internals->Closed->setChecked(loopClosed);
  if (loopClosed)
  {
    this->ModifyMode();
    Q_EMIT this->contourLoopClosed();
  }
}

void qtArcWidget::closeLoop(bool val)
{
  vtkSMNewWidgetRepresentationProxy* widget = this->widgetProxy();
  if (widget)
  {
    vtkSMProxy* repProxy = widget->GetRepresentationProxy();
    vtkSMPropertyHelper loopHelper(repProxy, "ClosedLoop");
    if (val)
    {
      widget->InvokeCommand("CloseLoop");
    }
    this->Internals->ModifyMode->setChecked(val);
    loopHelper.Set(val);
    repProxy->UpdateVTKObjects();
    this->render();
  }
}

void qtArcWidget::ModifyMode()
{
  this->Internals->ModifyMode->setChecked(true);
}

void qtArcWidget::checkCanBeEdited()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->widgetProxy();
  if (widget)
  {
    vtkSMProxy* repProxy = widget->GetRepresentationProxy();
    vtkSMPropertyHelper canEditHelper(repProxy, "CanEditInfo");
    canEditHelper.UpdateValueFromServer();
    int canEdit = canEditHelper.GetAsInt();
    if (!canEdit)
    {
      this->Internals->ModifyMode->setChecked(true);
    }
    this->Internals->EditMode->setVisible(canEdit);
    this->Internals->Closed->setVisible(canEdit);
  }
}

void qtArcWidget::updateMode()
{
  // the text should always be updated to this.
  vtkSMNewWidgetRepresentationProxy* widget = this->widgetProxy();
  if (widget)
  {
    if (this->Internals->EditMode->isChecked())
    {
      vtkSMPropertyHelper(widget, "WidgetState").Set(1);
    }
    else if (this->Internals->ModifyMode->isChecked())
    {
      vtkSMPropertyHelper(widget, "WidgetState").Set(2);
    }
    widget->UpdateVTKObjects();
  }
}

void qtArcWidget::finishContour()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->widgetProxy();
  widget->GetWidget()->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  Q_EMIT this->contourDone();
}

vtkSMProxy* qtArcWidget::pointPlacer() const
{
  return vtkSMPropertyHelper(this->widgetProxy(), "PointPlacer").GetAsProxy();
}

void qtArcWidget::reset()
{
  // update our mode
  this->Internals->EditMode->setDisabled(false);
  this->Internals->EditMode->setChecked(true);
  this->Internals->Closed->blockSignals(true);
  this->Internals->Closed->setEnabled(true);
  this->Internals->Closed->setChecked(false);
  // consistent with Closed checkbox
  this->closeLoop(false);
  this->Internals->Closed->blockSignals(false);
}

void qtArcWidget::setLineColor(const QColor& color)
{
  vtkSMProxy* widget = this->widgetProxy();
  vtkSMPropertyHelper(widget, "LineColor").Set(0, color.redF());
  vtkSMPropertyHelper(widget, "LineColor").Set(1, color.greenF());
  vtkSMPropertyHelper(widget, "LineColor").Set(2, color.blueF());
  widget->UpdateVTKObjects();
}

void qtArcWidget::useArcEditingUI(bool isWholeArc)
{
  this->Internals->Delete->setVisible(false);
  this->Internals->Closed->setEnabled(isWholeArc);
}

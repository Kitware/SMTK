//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqDiskPropertyWidget.h"
#include "smtk/extension/paraview/widgets/pqPointPickingVisibilityHelper.h"
#include "smtk/extension/paraview/widgets/ui_pqDiskPropertyWidget.h"

#include "pqCoreUtilities.h"
#include "pqPointPickingHelper.h"
#include "pqView.h"

#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"

#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkVector.h"
#include "vtkVersionMacros.h"

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 5, 0)
#include "vtkVectorOperators.h"
#endif

#include <QCheckBox>

class pqDiskPropertyWidget::Internals
{
public:
  Internals() = default;

  Ui::DiskPropertyWidget Ui;
};

pqDiskPropertyWidget::pqDiskPropertyWidget(
  vtkSMProxy* smproxy,
  vtkSMPropertyGroup* smgroup,
  QWidget* parentObj)
  : Superclass("representations", "DiskWidgetRepresentation", smproxy, smgroup, parentObj)
  , m_p(new pqDiskPropertyWidget::Internals())
{
  Ui::DiskPropertyWidget& ui = m_p->Ui;
  ui.setupUi(this);

  // link show3DWidget checkbox
  QObject::connect(
    ui.show3DWidget, &QCheckBox::toggled, this, &pqDiskPropertyWidget::setWidgetVisible);
  QObject::connect(
    this, &pqDiskPropertyWidget::widgetVisibilityToggled, ui.show3DWidget, &QCheckBox::setChecked);
  this->setWidgetVisible(ui.show3DWidget->isChecked());

#ifdef Q_OS_MAC
  ui.pickLabel->setText(ui.pickLabel->text().replace("Ctrl", "Cmd"));
#endif

  if (vtkSMProperty* ctr = smgroup->GetProperty("CenterPoint"))
  {
    ui.labelCenter->setText(tr(ctr->GetXMLLabel()));
    this->addPropertyLink(ui.centerX, "text2", SIGNAL(textChangedAndEditingFinished()), ctr, 0);
    this->addPropertyLink(ui.centerY, "text2", SIGNAL(textChangedAndEditingFinished()), ctr, 1);
    this->addPropertyLink(ui.centerZ, "text2", SIGNAL(textChangedAndEditingFinished()), ctr, 2);
    ui.labelCenter->setText(ctr->GetXMLLabel());
  }
  else
  {
    qCritical("Missing required property for function 'CenterPoint'.");
  }

  if (vtkSMProperty* nrm = smgroup->GetProperty("Normal"))
  {
    ui.labelNormal->setText(tr(nrm->GetXMLLabel()));
    this->addPropertyLink(ui.normalX, "text2", SIGNAL(textChangedAndEditingFinished()), nrm, 0);
    this->addPropertyLink(ui.normalY, "text2", SIGNAL(textChangedAndEditingFinished()), nrm, 1);
    this->addPropertyLink(ui.normalZ, "text2", SIGNAL(textChangedAndEditingFinished()), nrm, 2);
    ui.labelNormal->setText(nrm->GetXMLLabel());
  }
  else
  {
    qCritical("Missing required property for function 'Normal'.");
  }

  if (vtkSMProperty* rad = smgroup->GetProperty("Radius"))
  {
    ui.labelRadius->setText(tr(rad->GetXMLLabel()));
    this->addPropertyLink(ui.radius, "text2", SIGNAL(textChangedAndEditingFinished()), rad, 0);
  }
  else
  {
    qCritical("Missing required property for function 'Radius'.");
  }

  pqPointPickingHelper* pickHelper = new pqPointPickingHelper(QKeySequence(tr("P")), false, this);
  QObject::connect(
    this, &pqDiskPropertyWidget::viewChanged, pickHelper, &pqPointPickingHelper::setView);
  QObject::connect(pickHelper, &pqPointPickingHelper::pick, this, &pqDiskPropertyWidget::pick);
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper };

  pqPointPickingHelper* pickHelper2 =
    new pqPointPickingHelper(QKeySequence(tr("Ctrl+P")), true, this);
  QObject::connect(
    this, &pqDiskPropertyWidget::viewChanged, pickHelper2, &pqPointPickingHelper::setView);
  QObject::connect(pickHelper2, &pqPointPickingHelper::pick, this, &pqDiskPropertyWidget::pick);
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper2 };

  pqCoreUtilities::connect(
    this->widgetProxy(), vtkCommand::PropertyModifiedEvent, this, SLOT(updateInformationLabels()));
  this->updateInformationLabels();
}

pqDiskPropertyWidget::~pqDiskPropertyWidget() = default;

void pqDiskPropertyWidget::pick(double wx, double wy, double wz)
{
  double position[3] = { wx, wy, wz };
  vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "CenterPoint").Set(position, 3);
  wdgProxy->UpdateVTKObjects();
  Q_EMIT this->changeAvailable();
  this->render();
}

void pqDiskPropertyWidget::updateInformationLabels()
{
  // Ui::DiskPropertyWidget& ui = m_p->Ui;

  vtkVector3d ctr, nrm;
  vtkSMProxy* wproxy = this->widgetProxy();
  vtkSMPropertyHelper(wproxy, "CenterPoint").Get(ctr.GetData(), 3);
  vtkSMPropertyHelper(wproxy, "Normal").Get(nrm.GetData(), 3);
}

void pqDiskPropertyWidget::placeWidget()
{
  // Nothing to do?
}

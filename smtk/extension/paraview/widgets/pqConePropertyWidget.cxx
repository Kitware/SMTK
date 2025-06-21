//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqConePropertyWidget.h"
#include "smtk/extension/paraview/widgets/pqPointPickingVisibilityHelper.h"
#include "smtk/extension/paraview/widgets/ui_pqConePropertyWidget.h"

#include "pqCoreUtilities.h"
#include "pqPointPickingHelper.h"

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

class pqConePropertyWidget::Internals
{
public:
  Internals()
    : BottomRadiusName("Bottom radius")
  {
  }

  Ui::ConePropertyWidget Ui;
  bool PickPoint1{ true };
  std::string BottomRadiusName;
};

pqConePropertyWidget::pqConePropertyWidget(
  vtkSMProxy* smproxy,
  vtkSMPropertyGroup* smgroup,
  QWidget* parentObj)
  : Superclass("representations", "ConeWidgetRepresentation", smproxy, smgroup, parentObj)
  , m_p(new pqConePropertyWidget::Internals())
{
  Ui::ConePropertyWidget& ui = m_p->Ui;
  ui.setupUi(this);

  auto* topRad = smgroup->GetProperty("TopRadius");
  if (!topRad)
  {
    // Only show a single radius when both must be identical.
    ui.radius2->hide();
    ui.labelRadius2->hide();
    ui.labelRadius1->setText("Radius");
    ui.show3DWidget->setText("Show cylinder");
    ui.cylindrical->hide(); // We are forced into cylinder model
    auto* cyl = smgroup->GetProperty("Cylindrical");
    if (cyl)
    {
      int on = 1;
      vtkSMPropertyHelper(cyl).Set(&on, 1);
    }
  }

  // link show3DWidget checkbox
  this->connect(ui.show3DWidget, SIGNAL(toggled(bool)), SLOT(setWidgetVisible(bool)));
  ui.show3DWidget->connect(this, SIGNAL(widgetVisibilityToggled(bool)), SLOT(setChecked(bool)));
  this->setWidgetVisible(ui.show3DWidget->isChecked());

  // link show3DWidget checkbox
  this->connect(ui.cylindrical, SIGNAL(toggled(bool)), SLOT(setCylindrical(bool)));
  // ui.cylindrical->connect(this, SIGNAL(widgetVisibilityToggled(bool)), SLOT(setChecked(bool)));
  this->setCylindrical(ui.cylindrical->isChecked());

#ifdef Q_OS_MAC
  ui.pickLabel->setText(ui.pickLabel->text().replace("Ctrl", "Cmd"));
#endif

  if (vtkSMProperty* p1 = smgroup->GetProperty("BottomPoint"))
  {
    ui.labelPoint1->setText(tr(p1->GetXMLLabel()));
    this->addPropertyLink(ui.point1X, "text2", SIGNAL(textChangedAndEditingFinished()), p1, 0);
    this->addPropertyLink(ui.point1Y, "text2", SIGNAL(textChangedAndEditingFinished()), p1, 1);
    this->addPropertyLink(ui.point1Z, "text2", SIGNAL(textChangedAndEditingFinished()), p1, 2);
    ui.labelPoint1->setText(p1->GetXMLLabel());
  }
  else
  {
    qCritical("Missing required property for function 'BottomPoint'.");
  }

  if (vtkSMProperty* p2 = smgroup->GetProperty("TopPoint"))
  {
    ui.labelPoint2->setText(tr(p2->GetXMLLabel()));
    this->addPropertyLink(ui.point2X, "text2", SIGNAL(textChangedAndEditingFinished()), p2, 0);
    this->addPropertyLink(ui.point2Y, "text2", SIGNAL(textChangedAndEditingFinished()), p2, 1);
    this->addPropertyLink(ui.point2Z, "text2", SIGNAL(textChangedAndEditingFinished()), p2, 2);
    ui.labelPoint2->setText(p2->GetXMLLabel());
  }
  else
  {
    qCritical("Missing required property for function 'TopPoint'.");
  }

  if (vtkSMProperty* r1 = smgroup->GetProperty("BottomRadius"))
  {
    ui.labelRadius1->setText(tr(r1->GetXMLLabel()));
    this->addPropertyLink(ui.radius1, "text2", SIGNAL(textChangedAndEditingFinished()), r1, 0);
  }
  else
  {
    qCritical("Missing required property for function 'BottomRadius'.");
  }

  if (vtkSMProperty* r2 = smgroup->GetProperty("TopRadius"))
  {
    ui.labelRadius2->setText(tr(r2->GetXMLLabel()));
    this->addPropertyLink(ui.radius2, "text2", SIGNAL(textChangedAndEditingFinished()), r2, 0);
  }
  else
  {
    qCritical("Missing required property for function 'TopRadius'.");
  }

  pqPointPickingHelper* pickHelper = new pqPointPickingHelper(QKeySequence(tr("P")), false, this);
  pickHelper->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  this->connect(
    pickHelper, SIGNAL(pick(double, double, double)), SLOT(pick(double, double, double)));
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper };

  pqPointPickingHelper* pickHelper2 =
    new pqPointPickingHelper(QKeySequence(tr("Ctrl+P")), true, this);
  pickHelper2->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  this->connect(
    pickHelper2, SIGNAL(pick(double, double, double)), SLOT(pick(double, double, double)));
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper2 };

  pqPointPickingHelper* pickHelper3 = new pqPointPickingHelper(QKeySequence(tr("1")), false, this);
  pickHelper3->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  this->connect(
    pickHelper3, SIGNAL(pick(double, double, double)), SLOT(pickPoint1(double, double, double)));
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper3 };

  pqPointPickingHelper* pickHelper4 =
    new pqPointPickingHelper(QKeySequence(tr("Ctrl+1")), true, this);
  pickHelper4->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  this->connect(
    pickHelper4, SIGNAL(pick(double, double, double)), SLOT(pickPoint1(double, double, double)));
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper4 };

  pqPointPickingHelper* pickHelper5 = new pqPointPickingHelper(QKeySequence(tr("2")), false, this);
  pickHelper5->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  this->connect(
    pickHelper5, SIGNAL(pick(double, double, double)), SLOT(pickPoint2(double, double, double)));
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper5 };

  pqPointPickingHelper* pickHelper6 =
    new pqPointPickingHelper(QKeySequence(tr("Ctrl+2")), true, this);
  pickHelper6->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  this->connect(
    pickHelper6, SIGNAL(pick(double, double, double)), SLOT(pickPoint2(double, double, double)));
  pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *pickHelper6 };

  pqCoreUtilities::connect(
    this->widgetProxy(), vtkCommand::PropertyModifiedEvent, this, SLOT(updateInformationLabels()));
  this->updateInformationLabels();
}

pqConePropertyWidget::~pqConePropertyWidget() = default;

void pqConePropertyWidget::pick(double wx, double wy, double wz)
{
  if (m_p->PickPoint1)
  {
    this->pickPoint1(wx, wy, wz);
  }
  else
  {
    this->pickPoint2(wx, wy, wz);
  }
  m_p->PickPoint1 = !m_p->PickPoint1;
}

void pqConePropertyWidget::pickPoint1(double wx, double wy, double wz)
{
  double position[3] = { wx, wy, wz };
  vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "BottomPoint").Set(position, 3);
  wdgProxy->UpdateVTKObjects();
  Q_EMIT this->changeAvailable();
  this->render();
}

void pqConePropertyWidget::pickPoint2(double wx, double wy, double wz)
{
  double position[3] = { wx, wy, wz };
  vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "TopPoint").Set(position, 3);
  wdgProxy->UpdateVTKObjects();
  Q_EMIT this->changeAvailable();
  this->render();
}

void pqConePropertyWidget::setCylindrical(bool isCylinder)
{
  int cyl = isCylinder ? 1 : 0;
  vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "Cylindrical").Set(&cyl, 1);
  if (isCylinder)
  {
    m_p->BottomRadiusName = m_p->Ui.labelRadius1->text().toStdString();
    m_p->Ui.radius2->hide();
    m_p->Ui.labelRadius2->hide();
    m_p->Ui.labelRadius1->setText("Radius");
    m_p->Ui.show3DWidget->setText("Show cylinder");
  }
  else
  {
    m_p->Ui.radius2->show();
    m_p->Ui.labelRadius2->show();
    m_p->Ui.labelRadius1->setText(m_p->BottomRadiusName.c_str());
    m_p->Ui.show3DWidget->setText("Show cone");
  }
  wdgProxy->UpdateVTKObjects();
  Q_EMIT this->changeAvailable();
  this->render();
}

void pqConePropertyWidget::setForceCylindrical(bool isCylinder)
{
  m_p->Ui.cylindrical->setChecked(isCylinder);
  if (isCylinder)
  {
    m_p->Ui.cylindrical->hide();
  }
  else
  {
    m_p->Ui.cylindrical->show();
  }
  this->setCylindrical(isCylinder);
}

void pqConePropertyWidget::updateInformationLabels()
{
  Ui::ConePropertyWidget& ui = m_p->Ui;

  vtkVector3d p1, p2;
  vtkSMProxy* wproxy = this->widgetProxy();
  vtkSMPropertyHelper(wproxy, "BottomPoint").Get(p1.GetData(), 3);
  vtkSMPropertyHelper(wproxy, "TopPoint").Get(p2.GetData(), 3);

  double distance = (p2 - p1).Norm();
  ui.labelLength->setText(QString("<b>Length:</b> <i>%1</i> ").arg(distance));
}

void pqConePropertyWidget::placeWidget()
{
  // Nothing to do?
}

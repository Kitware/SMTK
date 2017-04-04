//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/qtLineWidget.h"
#include "ui_qtLineWidget.h"

#include "pqPropertyLinks.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"

class qtLineWidget::qtInternals {
public:
  Ui::qtLineWidget UI;
  pqPropertyLinks Links;
};

qtLineWidget::qtLineWidget(QWidget *parentWdg)
    : Superclass(qtInteractionWidget::createWidget("representations",
                                                   "LineWidgetRepresentation"),
                 parentWdg),
      Internals(new qtLineWidget::qtInternals()) {
  Ui::qtLineWidget &ui(this->Internals->UI);

  ui.setupUi(this);
  ui.visible->setChecked(this->isInteractivityEnabled());
  QObject::connect(ui.visible, SIGNAL(toggled(bool)), this,
                   SLOT(setEnableInteractivity(bool)));
  ui.visible->connect(this, SIGNAL(enableInteractivityChanged(bool)),
                      SLOT(setChecked(bool)));

  // Setup validators for all line widgets.
  QDoubleValidator *validator = new QDoubleValidator(this);
  ui.point1X->setValidator(validator);
  ui.point1Y->setValidator(validator);
  ui.point1Z->setValidator(validator);
  ui.point2X->setValidator(validator);
  ui.point2Y->setValidator(validator);
  ui.point2Z->setValidator(validator);

  vtkSMProxy *wdgProxy = this->widgetProxy();
  pqPropertyLinks &links = this->Internals->Links;
  if (vtkSMProperty *p1 = wdgProxy->GetProperty("Point1WorldPosition")) {
    ui.labelPoint1->setText(tr(p1->GetXMLLabel()));
    links.addPropertyLink(ui.point1X, "text2",
                          SIGNAL(textChangedAndEditingFinished()), wdgProxy, p1,
                          0);
    links.addPropertyLink(ui.point1Y, "text2",
                          SIGNAL(textChangedAndEditingFinished()), wdgProxy, p1,
                          1);
    links.addPropertyLink(ui.point1Z, "text2",
                          SIGNAL(textChangedAndEditingFinished()), wdgProxy, p1,
                          2);
  } else {
    qCritical("Missing required property for function 'Point1WorldPosition'.");
  }

  if (vtkSMProperty *p2 = wdgProxy->GetProperty("Point2WorldPosition")) {
    ui.labelPoint2->setText(tr(p2->GetXMLLabel()));
    links.addPropertyLink(ui.point2X, "text2",
                          SIGNAL(textChangedAndEditingFinished()), wdgProxy, p2,
                          0);
    links.addPropertyLink(ui.point2Y, "text2",
                          SIGNAL(textChangedAndEditingFinished()), wdgProxy, p2,
                          1);
    links.addPropertyLink(ui.point2Z, "text2",
                          SIGNAL(textChangedAndEditingFinished()), wdgProxy, p2,
                          2);
  } else {
    qCritical("Missing required property for function 'Point2WorldPosition'.");
  }

  QObject::connect(ui.xAxis, SIGNAL(clicked()), this, SLOT(xAxis()));
  QObject::connect(ui.yAxis, SIGNAL(clicked()), this, SLOT(yAxis()));
  QObject::connect(ui.zAxis, SIGNAL(clicked()), this, SLOT(zAxis()));
  this->connect(&links, SIGNAL(qtWidgetChanged()), SLOT(render()));
}

qtLineWidget::~qtLineWidget() {}

void qtLineWidget::setLineColor(const QColor &acolor) {
  double dcolor[3] = {acolor.redF(), acolor.greenF(), acolor.blueF()};
  if (vtkSMProxy *proxy = this->widgetProxy()) {
    vtkSMPropertyHelper(proxy, "LineColor").Set(dcolor, 3);
    proxy->UpdateVTKObjects();
    this->render();
  }
}

QColor qtLineWidget::color() const {
  if (vtkSMProxy *proxy = this->widgetProxy()) {
    double dcolor[3];
    vtkSMPropertyHelper(proxy, "LineColor").Get(dcolor, 3);
    return QColor::fromRgbF(dcolor[0], dcolor[1], dcolor[2]);
  }

  return QColor();
}

void qtLineWidget::setPoints(const double p1[3], const double p2[3]) {
  vtkSMProxy *wdg = this->widgetProxy();
  Q_ASSERT(wdg);

  vtkSMPropertyHelper(wdg, "Point1WorldPosition").Set(p1, 3);
  vtkSMPropertyHelper(wdg, "Point2WorldPosition").Set(p2, 3);
  wdg->UpdateVTKObjects();
  this->render();
}

void qtLineWidget::points(double p1[3], double p2[3]) const {
  vtkSMProxy *wdg = this->widgetProxy();
  Q_ASSERT(wdg);
  vtkSMPropertyHelper(wdg, "Point1WorldPosition").Get(p1, 3);
  vtkSMPropertyHelper(wdg, "Point2WorldPosition").Get(p2, 3);
}

void qtLineWidget::xAxis() {}

void qtLineWidget::yAxis() {}

void qtLineWidget::zAxis() {}

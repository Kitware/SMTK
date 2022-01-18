//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/extension/paraview/widgets/pqSlicePropertyWidgetBase.h"
#include "smtk/extension/paraview/widgets/pqSlicePropertyWidgetBaseP.h"

#include "pqActiveObjects.h"
#include "pqArraySelectorPropertyWidget.h"
#include "pqColorChooserButton.h"
#include "pqDataRepresentation.h"
#include "pqPointPickingHelper.h"
#include "pqRenderView.h"
#include "vtkCamera.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMTransferFunctionManager.h"

namespace
{
// Display sized implicit plane widget does not like it when any of the dimensions is 0. So
// we ensure that each dimension has some thickness. Then we scale the bounds
// by the given factor
void pqAdjustBounds(vtkBoundingBox& bbox, double scaleFactor)
{
  double max_length = bbox.GetMaxLength();
  max_length = max_length > 0 ? max_length * 0.05 : 1;
  double min_point[3], max_point[3];
  bbox.GetMinPoint(min_point[0], min_point[1], min_point[2]);
  bbox.GetMaxPoint(max_point[0], max_point[1], max_point[2]);
  for (int cc = 0; cc < 3; cc++)
  {
    if (bbox.GetLength(cc) == 0)
    {
      min_point[cc] -= max_length;
      max_point[cc] += max_length;
    }

    double mid = (min_point[cc] + max_point[cc]) / 2.0;
    min_point[cc] = mid + scaleFactor * (min_point[cc] - mid);
    max_point[cc] = mid + scaleFactor * (max_point[cc] - mid);
  }
  bbox.SetMinPoint(min_point);
  bbox.SetMaxPoint(max_point);
}
} // namespace

pqSlicePropertyWidgetBaseP::pqSlicePropertyWidgetBaseP(
  pqSlicePropertyWidgetBase* self,
  pqPipelineSource* inputData,
  vtkSMProxy* smproxy,
  vtkSMPropertyGroup* smgroup,
  QWidget* parentObject)
{
  (void)smproxy;
  (void)parentObject;
  this->setupUi(self);
  this->input = inputData;
  if (vtkSMProperty* origin = smgroup->GetProperty("Origin"))
  {
    self->addPropertyLink(
      this->originX, "text2", SIGNAL(textChangedAndEditingFinished()), origin, 0);
    self->addPropertyLink(
      this->originY, "text2", SIGNAL(textChangedAndEditingFinished()), origin, 1);
    self->addPropertyLink(
      this->originZ, "text2", SIGNAL(textChangedAndEditingFinished()), origin, 2);
    this->labelOrigin->setText(origin->GetXMLLabel());
    this->pickLabel->setText(
      this->pickLabel->text().replace("'Origin'", QString("'%1'").arg(origin->GetXMLLabel())));
    QString tooltip = pqSlicePropertyWidgetBase::getTooltip(origin);
    this->originX->setToolTip(tooltip);
    this->originY->setToolTip(tooltip);
    this->originZ->setToolTip(tooltip);
    this->labelOrigin->setToolTip(tooltip);
  }
  else
  {
    qCritical("Missing required property for function 'Origin'.");
  }

  if (vtkSMProperty* normal = smgroup->GetProperty("Normal"))
  {
    self->addPropertyLink(
      this->normalX, "text2", SIGNAL(textChangedAndEditingFinished()), normal, 0);
    self->addPropertyLink(
      this->normalY, "text2", SIGNAL(textChangedAndEditingFinished()), normal, 1);
    self->addPropertyLink(
      this->normalZ, "text2", SIGNAL(textChangedAndEditingFinished()), normal, 2);
    this->labelNormal->setText(normal->GetXMLLabel());
    QString tooltip = pqSlicePropertyWidgetBase::getTooltip(normal);
    this->normalX->setToolTip(tooltip);
    this->normalY->setToolTip(tooltip);
    this->normalZ->setToolTip(tooltip);
    this->labelNormal->setToolTip(tooltip);
  }
  else
  {
    qCritical("Missing required property for function 'Normal'.");
  }

  if (
    vtkSMIntVectorProperty* alwaysSnapToNearestAxis =
      vtkSMIntVectorProperty::SafeDownCast(smgroup->GetProperty("AlwaysSnapToNearestAxis")))
  {
    if (alwaysSnapToNearestAxis->GetNumberOfElements())
    {
      vtkSMNewWidgetRepresentationProxy* wdgProxy = self->widgetProxy();
      vtkSMPropertyHelper(wdgProxy, "AlwaysSnapToNearestAxis")
        .Set(alwaysSnapToNearestAxis->GetElements()[0]);
    }
  }

  // link a few buttons
  self->connect(this->useXNormal, SIGNAL(clicked()), SLOT(useXNormal()));
  self->connect(this->useYNormal, SIGNAL(clicked()), SLOT(useYNormal()));
  self->connect(this->useZNormal, SIGNAL(clicked()), SLOT(useZNormal()));
  self->connect(this->useCameraNormal, SIGNAL(clicked()), SLOT(useCameraNormal()));
  self->connect(this->resetCameraToNormal, SIGNAL(clicked()), SLOT(resetCameraToNormal()));
  self->connect(this->resetToDataBounds, SIGNAL(clicked()), SLOT(resetToDataBounds()));
  self->connect(this->showHandles, SIGNAL(toggled(bool)), SLOT(setDrawHandles(bool)));

  // link show3DWidget checkbox
  self->connect(this->show3DWidget, SIGNAL(toggled(bool)), SLOT(setWidgetVisible(bool)));
  this->show3DWidget->connect(self, SIGNAL(widgetVisibilityToggled(bool)), SLOT(setChecked(bool)));
  self->setWidgetVisible(this->show3DWidget->isChecked());

  using PickOption = pqPointPickingHelper::PickOption;

  // picking origin point actions
  pqPointPickingHelper* pickPointHelper =
    new pqPointPickingHelper(QKeySequence(QObject::tr("P")), false, self, PickOption::Coordinates);
  pickPointHelper->connect(self, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  self->connect(
    pickPointHelper, SIGNAL(pick(double, double, double)), SLOT(setOrigin(double, double, double)));

  pqPointPickingHelper* pickPointHelper2 = new pqPointPickingHelper(
    QKeySequence(QObject::tr("Ctrl+P")), true, self, PickOption::Coordinates);
  pickPointHelper2->connect(self, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  self->connect(
    pickPointHelper2,
    SIGNAL(pick(double, double, double)),
    SLOT(setOrigin(double, double, double)));

  // picking normal actions
  pqPointPickingHelper* pickNormalHelper =
    new pqPointPickingHelper(QKeySequence(QObject::tr("N")), false, self, PickOption::Normal);
  pickNormalHelper->connect(self, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  self->connect(
    pickNormalHelper,
    SIGNAL(pick(double, double, double)),
    SLOT(setNormal(double, double, double)));

  pqPointPickingHelper* pickNormalHelper2 =
    new pqPointPickingHelper(QKeySequence(QObject::tr("Ctrl+N")), true, self, PickOption::Normal);
  pickNormalHelper2->connect(self, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
  self->connect(
    pickNormalHelper2,
    SIGNAL(pick(double, double, double)),
    SLOT(setNormal(double, double, double)));

  // TODO: Are these needed?
  self->widgetProxy()->UpdateVTKObjects();
  self->widgetProxy()->UpdatePropertyInformation();

  this->colorBy = new pqDisplayColorWidget;
  QObject::connect(
    this->colorBy,
    &pqDisplayColorWidget::arraySelectionChanged,
    self,
    &pqSlicePropertyWidgetBase::updateColorByArray);
  this->buttonGrid->addWidget(this->colorBy, 2, 1, Qt::AlignCenter);

  QObject::connect(
    this->drawCrinkleEdges,
    &QCheckBox::toggled,
    self,
    &pqSlicePropertyWidgetBase::setDrawCrinkleEdges);
  self->setDrawCrinkleEdges(this->drawCrinkleEdges->checkState() == Qt::Checked);

  this->crinkleEdgeColor->setShowAlphaChannel(true);
  QColor defaultEdgeColor(80, 80, 80, 255);
  this->crinkleEdgeColor->setChosenColor(defaultEdgeColor); // Set the widget
  self->setCrinkleEdgeColor(defaultEdgeColor);              // Set the proxy property
  QObject::connect(
    this->crinkleEdgeColor,
    &pqColorChooserButton::chosenColorChanged,
    self,
    &pqSlicePropertyWidgetBase::setCrinkleEdgeColor);

  self->placeWidget();
}

pqSlicePropertyWidgetBase::pqSlicePropertyWidgetBase(
  const char* widgetGroup,
  const char* widgetName,
  pqPipelineSource* input,
  vtkSMProxy* smproxy,
  vtkSMPropertyGroup* smgroup,
  QWidget* parentObject)
  : Superclass(widgetGroup, widgetName, smproxy, smgroup, parentObject)
{
  m_p = new pqSlicePropertyWidgetBaseP(this, input, smproxy, smgroup, parentObject);
}

pqSlicePropertyWidgetBase::~pqSlicePropertyWidgetBase()
{
  delete m_p;
}

void pqSlicePropertyWidgetBase::placeWidget()
{
  vtkBoundingBox bbox = this->dataBounds();
  if (!bbox.IsValid())
  {
    return;
  }

  vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
  double scaleFactor = vtkSMPropertyHelper(wdgProxy, "PlaceFactor").GetAsDouble();
  pqAdjustBounds(bbox, scaleFactor);
  double bds[6];
  bbox.GetBounds(bds);
  vtkSMPropertyHelper(wdgProxy, "WidgetBounds").Set(bds, 6);
  wdgProxy->UpdateVTKObjects();
}

void pqSlicePropertyWidgetBase::setDrawPlane(bool val)
{
  vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "DrawPlane").Set(val ? 1 : 0);
  wdgProxy->UpdateVTKObjects();
  this->render();
}

void pqSlicePropertyWidgetBase::setDrawHandles(bool val)
{
  vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "DrawHandles").Set(val ? 1 : 0);
  wdgProxy->UpdateVTKObjects();
  this->render();
}

void pqSlicePropertyWidgetBase::apply()
{
  this->setDrawPlane(false);
  this->Superclass::apply();
}

void pqSlicePropertyWidgetBase::reset()
{
  this->setDrawPlane(false);
  this->Superclass::reset();
}

void pqSlicePropertyWidgetBase::resetToDataBounds()
{
  vtkBoundingBox bbox = this->dataBounds();

  if (bbox.IsValid())
  {
    vtkSMNewWidgetRepresentationProxy* wdgProxy = this->widgetProxy();
    double scaleFactor = vtkSMPropertyHelper(wdgProxy, "PlaceFactor").GetAsDouble();
    pqAdjustBounds(bbox, scaleFactor);
    double origin[3], bounds[6];
    bbox.GetCenter(origin);
    bbox.GetBounds(bounds);
    vtkSMPropertyHelper(wdgProxy, "Origin").Set(origin, 3);
    vtkSMPropertyHelper(wdgProxy, "WidgetBounds").Set(bounds, 6);
    wdgProxy->UpdateProperty("WidgetBounds", true);
    wdgProxy->UpdateVTKObjects();
    Q_EMIT this->changeAvailable();
    this->render();
  }
}

void pqSlicePropertyWidgetBase::resetCameraToNormal()
{
  if (pqRenderView* renView = qobject_cast<pqRenderView*>(this->view()))
  {
    vtkCamera* camera = renView->getRenderViewProxy()->GetActiveCamera();
    vtkSMProxy* wdgProxy = this->widgetProxy();
    double up[3], forward[3];
    camera->GetViewUp(up);
    vtkSMPropertyHelper(wdgProxy, "Normal").Get(forward, 3);
    vtkMath::Cross(up, forward, up);
    vtkMath::Cross(forward, up, up);
    renView->resetViewDirection(forward[0], forward[1], forward[2], up[0], up[1], up[2]);
    renView->render();
  }
}

void pqSlicePropertyWidgetBase::useCameraNormal()
{
  vtkSMRenderViewProxy* viewProxy =
    this->view() ? vtkSMRenderViewProxy::SafeDownCast(this->view()->getProxy()) : nullptr;
  if (viewProxy)
  {
    vtkCamera* camera = viewProxy->GetActiveCamera();

    double camera_normal[3];
    camera->GetViewPlaneNormal(camera_normal);
    camera_normal[0] = -camera_normal[0];
    camera_normal[1] = -camera_normal[1];
    camera_normal[2] = -camera_normal[2];
    this->setNormal(camera_normal[0], camera_normal[1], camera_normal[2]);
  }
}

void pqSlicePropertyWidgetBase::setView(pqView* view)
{
  this->Superclass::setView(view);
  if (view)
  {
    pqDataRepresentation* representation =
      m_p->input ? m_p->input->getRepresentation(view) : nullptr;
    if (representation)
    {
      m_p->colorBy->setRepresentation(representation);
    }
  }
}

void pqSlicePropertyWidgetBase::updateColorByArray()
{
  auto arrayFieldAndName = m_p->colorBy->arraySelection();
  std::string arrayName(arrayFieldAndName.second.toStdString());
  int arrayAssociation = arrayFieldAndName.first;
  int arrayComponent = m_p->colorBy->componentNumber();

  vtkSMProxy* wdgProxy = this->widgetProxy();

  vtkSMSessionProxyManager* pxm = pqActiveObjects::instance().proxyManager();
  auto* lkupProxy = m_p->transferFunctions->GetColorTransferFunction(arrayName.c_str(), pxm);
  wdgProxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(wdgProxy, "SliceLookupTable").Set(lkupProxy);
  vtkSMPropertyHelper(wdgProxy, "SliceColorArray")
    .SetInputArrayToProcess(arrayAssociation, arrayName.c_str());
  vtkSMPropertyHelper(wdgProxy, "SliceColorComponent").Set(arrayComponent);
  wdgProxy->UpdateVTKObjects();
}

void pqSlicePropertyWidgetBase::setDrawCrinkleEdges(bool visible)
{
  vtkSMProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "SliceEdgeVisibility").Set(visible ? 1 : 0);
  wdgProxy->UpdateVTKObjects();
  this->render();
}

void pqSlicePropertyWidgetBase::setCrinkleEdgeColor(const QColor& color)
{
  std::array<double, 4> rgba{ color.redF(), color.greenF(), color.blueF(), color.alphaF() };
  vtkSMProxy* wdgProxy = this->widgetProxy();
  vtkSMPropertyHelper(wdgProxy, "SliceEdgeColor").Set(rgba.data(), 4);
  wdgProxy->UpdateVTKObjects();
  this->render();
}

void pqSlicePropertyWidgetBase::setNormal(double wx, double wy, double wz)
{
  vtkSMProxy* wdgProxy = this->widgetProxy();
  double n[3] = { wx, wy, wz };
  vtkSMPropertyHelper(wdgProxy, "Normal").Set(n, 3);
  wdgProxy->UpdateVTKObjects();
  Q_EMIT this->changeAvailable();
  this->render();
}

void pqSlicePropertyWidgetBase::setOrigin(double wx, double wy, double wz)
{
  vtkSMProxy* wdgProxy = this->widgetProxy();
  double o[3] = { wx, wy, wz };
  vtkSMPropertyHelper(wdgProxy, "Origin").Set(o, 3);
  wdgProxy->UpdateVTKObjects();
  Q_EMIT this->changeAvailable();
  this->render();
}

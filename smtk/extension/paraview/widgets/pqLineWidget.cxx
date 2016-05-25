//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/widgets/pqLineWidget.h"

#include "pq3DWidgetFactory.h"
#include "pqApplicationCore.h"
#include "pqServerManagerModel.h"
#include "pqPropertyLinks.h"
#include "pqProxy.h"
#include "pqSMAdaptor.h"
#include "vtkEventQtSlotConnect.h"

#include "ui_qtLineWidget.h"

#include <QDoubleValidator>

#include <vtkMath.h>
#include <vtkPVDataInformation.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMSourceProxy.h>

/////////////////////////////////////////////////////////////////////////
// pqLineWidget::pqImplementation

class pqLineWidget::pqImplementation
{
public:
  pqImplementation() :
    WidgetPoint1(0),
    WidgetPoint2(0)
  {
    this->Links.setUseUncheckedProperties(false);
    this->Links.setAutoUpdateVTKObjects(true);
    this->PickPoint1 = true;
  }
  
  ~pqImplementation()
  {
  }
  
  /// Stores the Qt widgets
  Ui::qtLineWidget UI;
  
  /// Stores the 3D widget properties
  vtkSMDoubleVectorProperty* WidgetPoint1;
  vtkSMDoubleVectorProperty* WidgetPoint2;
  
  /// Maps Qt widgets to the 3D widget
  pqPropertyLinks Links;

  bool PickPoint1;
};

/////////////////////////////////////////////////////////////////////////
// pqLineWidget

pqLineWidget::pqLineWidget(vtkSMProxy* o, vtkSMProxy* pxy, QWidget* p,
  const char* xmlname/*="LineWidgetRepresentation"*/) :
  Superclass(o, pxy, p),
  Implementation(new pqImplementation())
{
  // enable picking.
  this->pickingSupported(QKeySequence(tr("P")));

  this->Implementation->UI.setupUi(this);
  this->Implementation->UI.visible->setChecked(this->widgetVisible());
  this->Implementation->UI.pickMeshPoint->setChecked(this->pickOnMeshPoint());

  // Setup validators for all line widgets.
  QDoubleValidator* validator = new QDoubleValidator(this);
  this->Implementation->UI.point1X->setValidator(validator);
  this->Implementation->UI.point1Y->setValidator(validator);
  this->Implementation->UI.point1Z->setValidator(validator);
  this->Implementation->UI.point2X->setValidator(validator);
  this->Implementation->UI.point2Y->setValidator(validator);
  this->Implementation->UI.point2Z->setValidator(validator);

  QObject::connect(this->Implementation->UI.visible,
    SIGNAL(toggled(bool)), this, SLOT(setWidgetVisible(bool)));

  QObject::connect(this->Implementation->UI.pickMeshPoint,
    SIGNAL(toggled(bool)), this, SLOT(setPickOnMeshPoint(bool)));

  QObject::connect(this, SIGNAL(widgetVisibilityChanged(bool)),
    this, SLOT(onWidgetVisibilityChanged(bool)));

  QObject::connect(this->Implementation->UI.xAxis,
    SIGNAL(clicked()), this, SLOT(onXAxis()));
  QObject::connect(this->Implementation->UI.yAxis,
    SIGNAL(clicked()), this, SLOT(onYAxis()));
  QObject::connect(this->Implementation->UI.zAxis,
    SIGNAL(clicked()), this, SLOT(onZAxis()));

  // Trigger a render when use explicitly edits the positions.
  QObject::connect(this->Implementation->UI.point1X, 
    SIGNAL(editingFinished()), 
    this, SLOT(render()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->UI.point1Y, 
    SIGNAL(editingFinished()), 
    this, SLOT(render()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->UI.point1Z,
    SIGNAL(editingFinished()), 
    this, SLOT(render()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->UI.point2X, 
    SIGNAL(editingFinished()), 
    this, SLOT(render()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->UI.point2Y, 
    SIGNAL(editingFinished()), 
    this, SLOT(render()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->UI.point2Z,
    SIGNAL(editingFinished()), 
    this, SLOT(render()), Qt::QueuedConnection);
  
  pqServerManagerModel* smmodel =
    pqApplicationCore::instance()->getServerManagerModel();
  
  this->createWidget(smmodel->findServer(o->GetSession()), xmlname);
  QObject::connect(&this->Implementation->Links, SIGNAL(qtWidgetChanged()),
    this, SLOT(setModified()));
}

//-----------------------------------------------------------------------------
pqLineWidget::~pqLineWidget()
{
  this->Implementation->Links.removeAllPropertyLinks();
  
  if(vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy())
    {
    pqApplicationCore::instance()->get3DWidgetFactory()->
      free3DWidget(widget);
      
    this->setWidgetProxy(0);
    }

  delete this->Implementation;
}

//-----------------------------------------------------------------------------
void pqLineWidget::setControlledProperties(vtkSMProperty* point1, vtkSMProperty* point2)
{
  this->Implementation->WidgetPoint1->Copy(point1);
  this->Implementation->WidgetPoint2->Copy(point2);

  // Map widget properties to controlled properties ...
  this->setControlledProperty("Point1WorldPosition", point1);
  this->setControlledProperty("Point2WorldPosition", point2);
}

//-----------------------------------------------------------------------------
void pqLineWidget::setControlledProperty(const char* function,
  vtkSMProperty* prop)
{
  this->Superclass::setControlledProperty(function, prop);
  if (QString("Point1WorldPosition") == function)
    {
    if (prop->GetXMLLabel())
      {
      this->Implementation->UI.labelPoint1->setText(
        prop->GetXMLLabel());
      }
    }
  else if (QString("Point2WorldPosition") == function)
    {
    if (prop->GetXMLLabel())
      {
      this->Implementation->UI.labelPoint2->setText(
        prop->GetXMLLabel());
      }
    }
}

//-----------------------------------------------------------------------------
void pqLineWidget::pick(double dx, double dy, double dz)
{
  vtkSMProxy* widget = this->getWidgetProxy();
  QList<QVariant> value;
  value << dx << dy << dz;

  bool point1;
  int pickPointIndex = this->Implementation->UI.pickPoint->currentIndex();
  if (pickPointIndex == 1)
    {
    point1 = true;
    }
  else if (pickPointIndex == 2)
    {
    point1 = false;
    }
  else
    {
    point1 = this->Implementation->PickPoint1;
    this->Implementation->PickPoint1 = 
      !this->Implementation->PickPoint1;
    }

  if (point1)
    {
    pqSMAdaptor::setMultipleElementProperty(
      widget->GetProperty("Point1WorldPosition"), value);
    }
  else
    {
    pqSMAdaptor::setMultipleElementProperty(
      widget->GetProperty("Point2WorldPosition"), value);
    }
  widget->UpdateVTKObjects();

  this->setModified();
  this->render();
}

//-----------------------------------------------------------------------------
void pqLineWidget::onXAxis()
{
  double object_center[3];
  double object_size[3];
  this->getReferenceBoundingBox(object_center, object_size);
       
  if(this->Implementation->WidgetPoint1 && this->Implementation->WidgetPoint2)
    {
    this->Implementation->WidgetPoint1->SetElement(0, object_center[0] - object_size[0] * 0.5);
    this->Implementation->WidgetPoint1->SetElement(1, object_center[1]);
    this->Implementation->WidgetPoint1->SetElement(2, object_center[2]);

    this->Implementation->WidgetPoint2->SetElement(0, object_center[0] + object_size[0] * 0.5);
    this->Implementation->WidgetPoint2->SetElement(1, object_center[1]);
    this->Implementation->WidgetPoint2->SetElement(2, object_center[2]);
  
    this->getWidgetProxy()->UpdateVTKObjects();
    pqApplicationCore::instance()->render();
    this->setModified();
    }
}

//-----------------------------------------------------------------------------
void pqLineWidget::onYAxis()
{
  double object_center[3];
  double object_size[3];
  this->getReferenceBoundingBox(object_center, object_size);
       
  if(this->Implementation->WidgetPoint1 && this->Implementation->WidgetPoint2)
    {
    this->Implementation->WidgetPoint1->SetElement(0, object_center[0]);
    this->Implementation->WidgetPoint1->SetElement(1, object_center[1] - object_size[1] * 0.5);
    this->Implementation->WidgetPoint1->SetElement(2, object_center[2]);

    this->Implementation->WidgetPoint2->SetElement(0, object_center[0]);
    this->Implementation->WidgetPoint2->SetElement(1, object_center[1] + object_size[1] * 0.5);
    this->Implementation->WidgetPoint2->SetElement(2, object_center[2]);
  
    this->getWidgetProxy()->UpdateVTKObjects();
    pqApplicationCore::instance()->render();
    this->setModified();
    }
}

//-----------------------------------------------------------------------------
void pqLineWidget::onZAxis()
{
  double object_center[3];
  double object_size[3];
  this->getReferenceBoundingBox(object_center, object_size);
       
  if(this->Implementation->WidgetPoint1 && this->Implementation->WidgetPoint2)
    {
    this->Implementation->WidgetPoint1->SetElement(0, object_center[0]);
    this->Implementation->WidgetPoint1->SetElement(1, object_center[1]);
    this->Implementation->WidgetPoint1->SetElement(2, object_center[2] - object_size[2] * 0.5);

    this->Implementation->WidgetPoint2->SetElement(0, object_center[0]);
    this->Implementation->WidgetPoint2->SetElement(1, object_center[1]);
    this->Implementation->WidgetPoint2->SetElement(2, object_center[2] + object_size[2] * 0.5);
  
    this->getWidgetProxy()->UpdateVTKObjects();
    pqApplicationCore::instance()->render();
    this->setModified();
    }
}

//-----------------------------------------------------------------------------
void pqLineWidget::createWidget(pqServer* server, const QString& xmlname)
{
  vtkSMNewWidgetRepresentationProxy* const widget =
    pqApplicationCore::instance()->get3DWidgetFactory()->get3DWidget(
      xmlname, server, this->getReferenceProxy());
  this->setWidgetProxy(widget);

  widget->UpdateVTKObjects();
  widget->UpdatePropertyInformation();

  this->Implementation->WidgetPoint1 = vtkSMDoubleVectorProperty::SafeDownCast(
    widget->GetProperty("Point1WorldPosition"));
  this->Implementation->WidgetPoint2 = vtkSMDoubleVectorProperty::SafeDownCast(
    widget->GetProperty("Point2WorldPosition"));

  this->Implementation->Links.addPropertyLink(
    this->Implementation->UI.point1X, "text2",
    SIGNAL(textChanged(const QString&)),
    widget, this->Implementation->WidgetPoint1, 0);

  this->Implementation->Links.addPropertyLink(
    this->Implementation->UI.point1Y, "text2",
    SIGNAL(textChanged(const QString&)),
    widget, this->Implementation->WidgetPoint1, 1);

  this->Implementation->Links.addPropertyLink(
    this->Implementation->UI.point1Z, "text2",
    SIGNAL(textChanged(const QString&)),
    widget, this->Implementation->WidgetPoint1, 2);

  this->Implementation->Links.addPropertyLink(
    this->Implementation->UI.point2X, "text2",
    SIGNAL(textChanged(const QString&)),
    widget, this->Implementation->WidgetPoint2, 0);

  this->Implementation->Links.addPropertyLink(
    this->Implementation->UI.point2Y, "text2",
    SIGNAL(textChanged(const QString&)),
    widget, this->Implementation->WidgetPoint2, 1);

  this->Implementation->Links.addPropertyLink(
    this->Implementation->UI.point2Z, "text2",
    SIGNAL(textChanged(const QString&)),
    widget, this->Implementation->WidgetPoint2, 2);

}

//-----------------------------------------------------------------------------
void pqLineWidget::resetBounds(double bounds[6])
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if(vtkSMDoubleVectorProperty* const place_widget =
    vtkSMDoubleVectorProperty::SafeDownCast(
      widget->GetProperty("PlaceWidget")))
    {
    place_widget->SetElements(bounds);
    widget->UpdateProperty("PlaceWidget", 1);
    }
  widget->UpdatePropertyInformation();
}

//-----------------------------------------------------------------------------
void pqLineWidget::getReferenceBoundingBox(double center[3], double sz[3])
{
  double input_bounds[6];
  vtkMath::UninitializeBounds(input_bounds);
  this->getReferenceInputBounds(input_bounds);
  
  if(vtkMath::AreBoundsInitialized(input_bounds))
    {
    center[0] = (input_bounds[0] + input_bounds[1]) / 2.0;
    center[1] = (input_bounds[2] + input_bounds[3]) / 2.0;
    center[2] = (input_bounds[4] + input_bounds[5]) / 2.0;

    // extended a bit
    sz[0] = fabs(input_bounds[1] - input_bounds[0]);
    sz[1] = fabs(input_bounds[3] - input_bounds[2]);
    sz[2] = fabs(input_bounds[5] - input_bounds[4]);
    }
  else if(this->Implementation->WidgetPoint1 &&
          this->Implementation->WidgetPoint2)
    {
    // get spherical bounds from what we had before
    double* tmp1 = this->Implementation->WidgetPoint1->GetElements();
    double* tmp2 = this->Implementation->WidgetPoint2->GetElements();
    center[0] = (tmp1[0] + tmp2[0])/2.0;
    center[1] = (tmp1[1] + tmp2[1])/2.0;
    center[2] = (tmp1[2] + tmp2[2])/2.0;
    sz[0] = fabs(tmp1[0] - tmp2[0]);
    sz[1] = fabs(tmp1[1] - tmp2[1]);
    sz[2] = fabs(tmp1[2] - tmp2[2]);
    double s = qMax(qMax(sz[0], sz[1]), sz[2]);
    sz[0] = s;
    sz[1] = s;
    sz[2] = s;
    }
}

//-----------------------------------------------------------------------------
void pqLineWidget::onWidgetVisibilityChanged(bool visible)
{
  this->Implementation->UI.visible->blockSignals(true);
  this->Implementation->UI.visible->setChecked(visible);
  this->Implementation->UI.visible->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqLineWidget::setLineColor(const QColor& color)
{
  vtkSMProxy* widget = this->getWidgetProxy();
  vtkSMPropertyHelper(widget,
    "LineColor").Set(0, color.redF());
 vtkSMPropertyHelper(widget,
    "LineColor").Set(1,color.greenF());
 vtkSMPropertyHelper(widget,
    "LineColor").Set(2 , color.blueF());
  widget->UpdateVTKObjects(); 
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKPointItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqHandlePropertyWidget.h"
#include "pqImplicitPlanePropertyWidget.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "pqSpherePropertyWidget.h"
#include "vtkPVXMLElement.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

using qtItem = smtk::extension::qtItem;
using AttributeItemInfo = smtk::extension::AttributeItemInfo;

pqSMTKPointItemWidget::pqSMTKPointItemWidget(
  const smtk::extension::AttributeItemInfo& info, Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKPointItemWidget::~pqSMTKPointItemWidget()
{
}

qtItem* pqSMTKPointItemWidget::createPointItemWidget(const AttributeItemInfo& info)
{
  return new pqSMTKPointItemWidget(info);
}

bool pqSMTKPointItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget)
{
  // I. Reject items we can't map to a point:
  smtk::attribute::DoubleItemPtr pointItem;
  if (!fetchPointItem(pointItem))
  {
    return false;
  }

  // II. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("smtk_widgets", "SMTKHandleWidget", server, "");
  if (!proxy)
  {
    return false;
  }
  widget = new pqHandlePropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  auto widgetProxy = widget->widgetProxy();
  vtkSMPropertyHelper(widgetProxy, "WorldPosition").Set(&(*pointItem->begin()), 3);

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKPointItemWidget::updateItemFromWidget()
{
  smtk::attribute::DoubleItemPtr pointItem;
  if (!fetchPointItem(pointItem))
  {
    return;
  }
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  // pqImplicitPlanePropertyWidget* pw = dynamic_cast<pqImplicitPlanePropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper pointHelper(widget, "WorldPosition");
  bool didChange = false;
  for (int i = 0; i < 3; ++i)
  {
    double coord = pointHelper.GetAsDouble(i);
    didChange |= (pointItem->value(i) != coord);
    pointItem->setValue(i, coord);
  }
  if (didChange)
  {
    emit modified();
  }
}

bool pqSMTKPointItemWidget::fetchPointItem(smtk::attribute::DoubleItemPtr& pointItem)
{
  pointItem = m_itemInfo.itemAs<smtk::attribute::DoubleItem>();
  if (!pointItem || pointItem->numberOfValues() != 3)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Expected a double item with 3 values.");
    return false;
  }
  return true;
}

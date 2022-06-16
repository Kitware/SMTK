//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKLineItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqLinePropertyWidget.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "vtkPVXMLElement.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

using qtItem = smtk::extension::qtItem;
using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

pqSMTKLineItemWidget::pqSMTKLineItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKLineItemWidget::~pqSMTKLineItemWidget() = default;

qtItem* pqSMTKLineItemWidget::createLineItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKLineItemWidget(info);
}

bool pqSMTKLineItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  //I. Reject items we can't map to a plane:
  smtk::attribute::DoubleItemPtr point1Item;
  smtk::attribute::DoubleItemPtr point2Item;
  if (!fetchEndpointItems(point1Item, point2Item))
  {
    return false;
  }

  // II. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("smtk_widgets", "SMTKLineWidget", server, "");
  if (!proxy)
  {
    return false;
  }
  widget = new pqLinePropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  auto* widgetProxy = widget->widgetProxy();
  vtkSMPropertyHelper(widgetProxy, "Point1WorldPosition").Set(&(*point1Item->begin()), 3);
  vtkSMPropertyHelper(widgetProxy, "Point2WorldPosition").Set(&(*point2Item->begin()), 3);

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
bool pqSMTKLineItemWidget::updateItemFromWidgetInternal()
{
  smtk::attribute::DoubleItemPtr point1Item;
  smtk::attribute::DoubleItemPtr point2Item;
  if (!fetchEndpointItems(point1Item, point2Item))
  {
    return false;
  }
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  // pqImplicitPlanePropertyWidget* pw = dynamic_cast<pqImplicitPlanePropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper point1Helper(widget, "Point1WorldPosition");
  vtkSMPropertyHelper point2Helper(widget, "Point2WorldPosition");
  bool didChange = false;
  for (int i = 0; i < 3; ++i)
  {
    double p1c = point1Helper.GetAsDouble(i);
    double p2c = point2Helper.GetAsDouble(i);
    didChange |= (point1Item->value(i) != p1c) || (point2Item->value(i) != p2c);
    point1Item->setValue(i, p1c);
    point2Item->setValue(i, p2c);
  }
  return didChange;
}

bool pqSMTKLineItemWidget::updateWidgetFromItemInternal()
{
  smtk::attribute::DoubleItemPtr point1Item;
  smtk::attribute::DoubleItemPtr point2Item;
  if (!fetchEndpointItems(point1Item, point2Item))
  {
    return false;
  }
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  // pqImplicitPlanePropertyWidget* pw = dynamic_cast<pqImplicitPlanePropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper point1Helper(widget, "Point1WorldPosition");
  vtkSMPropertyHelper point2Helper(widget, "Point2WorldPosition");
  bool didChange = false;
  for (int i = 0; i < 3; ++i)
  {
    double p1c = point1Helper.GetAsDouble(i);
    double p2c = point2Helper.GetAsDouble(i);
    didChange |= (point1Item->value(i) != p1c) || (point2Item->value(i) != p2c);
    point1Helper.Set(i, point1Item->value(i));
    point2Helper.Set(i, point2Item->value(i));
  }
  return didChange;
}

bool pqSMTKLineItemWidget::fetchEndpointItems(
  smtk::attribute::DoubleItemPtr& point1Item,
  smtk::attribute::DoubleItemPtr& point2Item)
{
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 2)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Expected a group item with 1 group of 2 items.");
    return false;
  }
  std::string point1ItemName;
  std::string point2ItemName;
  if (!m_itemInfo.component().attribute("Point1", point1ItemName))
  {
    point1ItemName = "Point1";
  }
  if (!m_itemInfo.component().attribute("Point2", point2ItemName))
  {
    point2ItemName = "Point2";
  }
  point1Item = groupItem->findAs<smtk::attribute::DoubleItem>(point1ItemName);
  point2Item = groupItem->findAs<smtk::attribute::DoubleItem>(point2ItemName);
  if (!point1Item || !point2Item)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find"
        << " a DoubleItem for Point1 named \"" << point1ItemName << "\","
        << " a DoubleItem for Point2 named \"" << point2ItemName << "\","
        << " or both.");
    return false;
  }
  if (point1Item->numberOfValues() != 3 || point2Item->numberOfValues() != 3)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "The point1 (" << point1Item->numberOfValues() << ")"
                     << " and point2 (" << point2Item->numberOfValues() << ")"
                     << " items must both have exactly 3 values.");
    return false;
  }
  return true;
}

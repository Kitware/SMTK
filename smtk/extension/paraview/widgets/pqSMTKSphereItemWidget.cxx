//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKSphereItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
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
using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

pqSMTKSphereItemWidget::pqSMTKSphereItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKSphereItemWidget::~pqSMTKSphereItemWidget() = default;

qtItem* pqSMTKSphereItemWidget::createSphereItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKSphereItemWidget(info);
}

bool pqSMTKSphereItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  //I. Reject items we can't map to a plane:
  smtk::attribute::DoubleItemPtr centerItem;
  smtk::attribute::DoubleItemPtr radiusItem;
  if (!fetchCenterAndRadiusItems(centerItem, radiusItem))
  {
    return false;
  }

  // II. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "Sphere", server, "");
  if (!proxy)
  {
    return false;
  }
  widget = new pqSpherePropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  auto* widgetProxy = widget->widgetProxy();
  vtkSMPropertyHelper(widgetProxy, "Center").Set(&(*centerItem->begin()), 3);
  vtkSMPropertyHelper(widgetProxy, "Radius").Set(&(*radiusItem->begin()), 1);

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKSphereItemWidget::updateItemFromWidgetInternal()
{
  smtk::attribute::DoubleItemPtr centerItem;
  smtk::attribute::DoubleItemPtr radiusItem;
  if (!fetchCenterAndRadiusItems(centerItem, radiusItem))
  {
    return;
  }
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  // pqImplicitPlanePropertyWidget* pw = dynamic_cast<pqImplicitPlanePropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper centerHelper(widget, "Center");
  vtkSMPropertyHelper radiusHelper(widget, "Radius");

  // NB: Because ParaView widgets may emit a "value possibly changed" signal when
  //     no value actually _has_ changed, we do not want to emit error messages
  //     that could frighten/confuse users just because the SMTK items were not
  //     updated. But this can be a silent failure point.
  bool didChange = false;
  for (int i = 0; i < 3; ++i)
  {
    double cv = centerHelper.GetAsDouble(i);
    didChange |= (centerItem->value(i) != cv);
    centerItem->setValue(i, cv);
  }
  double rv = radiusHelper.GetAsDouble(0);
  didChange |= (radiusItem->value(0) != rv);
  radiusItem->setValue(0, rv);
  if (didChange)
  {
    Q_EMIT modified();
  }
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKSphereItemWidget::updateWidgetFromItemInternal()
{
  smtk::attribute::DoubleItemPtr centerItem;
  smtk::attribute::DoubleItemPtr radiusItem;
  if (!fetchCenterAndRadiusItems(centerItem, radiusItem))
  {
    return;
  }
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  // pqImplicitPlanePropertyWidget* pw = dynamic_cast<pqImplicitPlanePropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper centerHelper(widget, "Center");
  vtkSMPropertyHelper radiusHelper(widget, "Radius");

  bool didChange = false;
  for (int i = 0; i < 3; ++i)
  {
    double cv = centerItem->value(i);
    didChange |= (centerHelper.GetAsDouble(i) != cv);
    centerHelper.Set(i, cv);
  }
  double rv = radiusItem->value(0);
  didChange |= (radiusHelper.GetAsDouble(0) != rv);
  radiusHelper.Set(rv);
}

bool pqSMTKSphereItemWidget::fetchCenterAndRadiusItems(
  smtk::attribute::DoubleItemPtr& centerItem,
  smtk::attribute::DoubleItemPtr& radiusItem)
{
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 2)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Expected a group item with 1 group of 2 items.");
    return false;
  }
  std::string centerItemName;
  std::string radiusItemName;
  if (!m_itemInfo.component().attribute("Center", centerItemName))
  {
    centerItemName = "Center";
  }
  if (!m_itemInfo.component().attribute("Radius", radiusItemName))
  {
    radiusItemName = "Radius";
  }
  centerItem = groupItem->findAs<smtk::attribute::DoubleItem>(centerItemName);
  radiusItem = groupItem->findAs<smtk::attribute::DoubleItem>(radiusItemName);
  if (!centerItem || !radiusItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find"
        << " a DoubleItem for the center named \"" << centerItemName << "\","
        << " a DoubleItem for the radius named \"" << radiusItemName << "\","
        << " or both.");
    return false;
  }
  if (centerItem->numberOfValues() != 3 || radiusItem->numberOfValues() != 1)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "The center (" << centerItem->numberOfValues() << ")"
                     << " and radius (" << radiusItem->numberOfValues() << ")"
                     << " must have exactly 3 and 1 values, respectively.");
    return false;
  }
  return true;
}

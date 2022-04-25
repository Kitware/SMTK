//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKPlaneItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqDisplaySizedImplicitPlanePropertyWidget.h"
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

pqSMTKPlaneItemWidget::pqSMTKPlaneItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKPlaneItemWidget::~pqSMTKPlaneItemWidget() = default;

qtItem* pqSMTKPlaneItemWidget::createPlaneItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKPlaneItemWidget(info);
}

bool pqSMTKPlaneItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  //I. Reject items we can't map to a plane:
  smtk::attribute::DoubleItemPtr originItem;
  smtk::attribute::DoubleItemPtr normalItem;
  if (!fetchOriginAndNormalItems(originItem, normalItem))
  {
    return false;
  }

  // II. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "Plane", server, "");
  if (!proxy)
  {
    return false;
  }
  widget = new pqDisplaySizedImplicitPlanePropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  auto* widgetProxy = widget->widgetProxy();
  vtkSMPropertyHelper(widgetProxy, "Origin").Set(&(*originItem->begin()), 3);
  vtkSMPropertyHelper(widgetProxy, "Normal").Set(&(*normalItem->begin()), 3);
  int drawOutline = 0;
  vtkSMPropertyHelper(widgetProxy, "DrawOutline").Set(drawOutline, 0);

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKPlaneItemWidget::updateItemFromWidgetInternal()
{
  smtk::attribute::DoubleItemPtr originItem;
  smtk::attribute::DoubleItemPtr normalItem;
  if (!fetchOriginAndNormalItems(originItem, normalItem))
  {
    return;
  }
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  // pqImplicitPlanePropertyWidget* pw = dynamic_cast<pqImplicitPlanePropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper originHelper(widget, "Origin");
  vtkSMPropertyHelper normalHelper(widget, "Normal");
  bool didChange = false;
  for (int i = 0; i < 3; ++i)
  {
    double ov = originHelper.GetAsDouble(i);
    double nv = normalHelper.GetAsDouble(i);
    didChange |= (originItem->value(i) != ov) || (normalItem->value(i) != nv);
    originItem->setValue(i, ov);
    normalItem->setValue(i, nv);
  }
  if (didChange)
  {
    Q_EMIT modified();
  }
}

bool pqSMTKPlaneItemWidget::fetchOriginAndNormalItems(
  smtk::attribute::DoubleItemPtr& originItem,
  smtk::attribute::DoubleItemPtr& normalItem)
{
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 2)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Expected a group item with 1 group of 2 items.");
    return false;
  }
  std::string originItemName;
  std::string normalItemName;
  if (!m_itemInfo.component().attribute("Origin", originItemName))
  {
    originItemName = "Origin";
  }
  if (!m_itemInfo.component().attribute("Normal", normalItemName))
  {
    normalItemName = "Normal";
  }
  originItem = groupItem->findAs<smtk::attribute::DoubleItem>(originItemName);
  normalItem = groupItem->findAs<smtk::attribute::DoubleItem>(normalItemName);
  if (!originItem || !normalItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find"
        << " a DoubleItem for the origin named \"" << originItemName << "\","
        << " a DoubleItem for the normal named \"" << normalItemName << "\","
        << " or both.");
    return false;
  }
  if (originItem->numberOfValues() != 3 || normalItem->numberOfValues() != 3)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "The origin (" << originItem->numberOfValues() << ")"
                     << " and normal (" << normalItem->numberOfValues() << ")"
                     << " must both have exactly 3 values.");
    return false;
  }
  return true;
}

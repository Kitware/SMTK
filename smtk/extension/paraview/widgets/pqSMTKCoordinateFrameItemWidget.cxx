//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKCoordinateFrameItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoordinateFramePropertyWidget.h"
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

pqSMTKCoordinateFrameItemWidget::pqSMTKCoordinateFrameItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKCoordinateFrameItemWidget::~pqSMTKCoordinateFrameItemWidget() = default;

qtItem* pqSMTKCoordinateFrameItemWidget::createCoordinateFrameItemWidget(
  const qtAttributeItemInfo& info)
{
  return new pqSMTKCoordinateFrameItemWidget(info);
}

bool pqSMTKCoordinateFrameItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  //I. Reject items we can't map to a plane:
  smtk::attribute::DoubleItemPtr originItem;
  smtk::attribute::DoubleItemPtr xAxisItem;
  smtk::attribute::DoubleItemPtr yAxisItem;
  smtk::attribute::DoubleItemPtr zAxisItem;
  if (!fetchOriginAndAxisItems(originItem, xAxisItem, yAxisItem, zAxisItem))
  {
    return false;
  }

  // II. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "CoordinateFrame", server, "");
  if (!proxy)
  {
    return false;
  }
  widget = new pqCoordinateFramePropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // II. Initialize the properties.
  auto* widgetProxy = widget->widgetProxy();
  vtkSMPropertyHelper(widgetProxy, "Origin").Set(&(*originItem->begin()), 3);
  vtkSMPropertyHelper(widgetProxy, "XAxis").Set(&(*xAxisItem->begin()), 3);
  vtkSMPropertyHelper(widgetProxy, "YAxis").Set(&(*yAxisItem->begin()), 3);
  vtkSMPropertyHelper(widgetProxy, "ZAxis").Set(&(*zAxisItem->begin()), 3);

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKCoordinateFrameItemWidget::updateItemFromWidgetInternal()
{
  smtk::attribute::DoubleItemPtr originItem;
  smtk::attribute::DoubleItemPtr xAxisItem;
  smtk::attribute::DoubleItemPtr yAxisItem;
  smtk::attribute::DoubleItemPtr zAxisItem;
  if (!fetchOriginAndAxisItems(originItem, xAxisItem, yAxisItem, zAxisItem))
  {
    return;
  }
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  // pqImplicitPlanePropertyWidget* pw = dynamic_cast<pqImplicitPlanePropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper originHelper(widget, "Origin");
  vtkSMPropertyHelper xAxisHelper(widget, "XAxis");
  vtkSMPropertyHelper yAxisHelper(widget, "YAxis");
  vtkSMPropertyHelper zAxisHelper(widget, "ZAxis");
  bool didChange = false;
  for (int i = 0; i < 3; ++i)
  {
    double ov = originHelper.GetAsDouble(i);
    double xv = xAxisHelper.GetAsDouble(i);
    double yv = yAxisHelper.GetAsDouble(i);
    double zv = zAxisHelper.GetAsDouble(i);
    didChange |= (originItem->value(i) != ov) || (xAxisItem->value(i) != xv) ||
      (yAxisItem->value(i) != yv) || (zAxisItem->value(i) != zv);
    originItem->setValue(i, ov);
    xAxisItem->setValue(i, xv);
    yAxisItem->setValue(i, yv);
    zAxisItem->setValue(i, zv);
  }
  if (didChange)
  {
    Q_EMIT modified();
  }
}

bool pqSMTKCoordinateFrameItemWidget::fetchOriginAndAxisItems(
  smtk::attribute::DoubleItemPtr& originItem,
  smtk::attribute::DoubleItemPtr& xAxisItem,
  smtk::attribute::DoubleItemPtr& yAxisItem,
  smtk::attribute::DoubleItemPtr& zAxisItem)
{
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 4)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Expected a group item with 1 group of 4 items.");
    return false;
  }
  std::string originItemName;
  std::string xAxisItemName;
  std::string yAxisItemName;
  std::string zAxisItemName;
  if (!m_itemInfo.component().attribute("Origin", originItemName))
  {
    originItemName = "Origin";
  }
  if (!m_itemInfo.component().attribute("XAxis", xAxisItemName))
  {
    xAxisItemName = "XAxis";
  }
  if (!m_itemInfo.component().attribute("YAxis", yAxisItemName))
  {
    yAxisItemName = "YAxis";
  }
  if (!m_itemInfo.component().attribute("ZAxis", zAxisItemName))
  {
    zAxisItemName = "ZAxis";
  }
  originItem = groupItem->findAs<smtk::attribute::DoubleItem>(originItemName);
  xAxisItem = groupItem->findAs<smtk::attribute::DoubleItem>(xAxisItemName);
  yAxisItem = groupItem->findAs<smtk::attribute::DoubleItem>(yAxisItemName);
  zAxisItem = groupItem->findAs<smtk::attribute::DoubleItem>(zAxisItemName);
  if (!originItem || !xAxisItem || !yAxisItem || !zAxisItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find some subset of"
        << " a DoubleItem for the origin named \"" << originItemName << "\","
        << " a DoubleItem for the x axis named \"" << xAxisItemName << "\","
        << " a DoubleItem for the y axis named \"" << yAxisItemName << "\","
        << " a DoubleItem for the z axis named \"" << zAxisItemName << ".");
    return false;
  }
  if (
    originItem->numberOfValues() != 3 || xAxisItem->numberOfValues() != 3 ||
    yAxisItem->numberOfValues() != 3 || zAxisItem->numberOfValues() != 3)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "The origin (" << originItem->numberOfValues() << ")"
                     << " and xAxis (" << xAxisItem->numberOfValues() << ")"
                     << " and yAxis (" << yAxisItem->numberOfValues() << ")"
                     << " and zAxis (" << zAxisItem->numberOfValues() << ")"
                     << " must all have exactly 3 values.");
    return false;
  }
  return true;
}

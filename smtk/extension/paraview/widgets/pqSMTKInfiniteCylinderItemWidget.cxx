//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKInfiniteCylinderItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCylinderPropertyWidget.h"
#include "pqDataRepresentation.h"
#include "pqImplicitPlanePropertyWidget.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "pqSpherePropertyWidget.h"
#include "vtkMath.h"
#include "vtkPVXMLElement.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkVector.h"
#include "vtkVectorOperators.h"

using qtItem = smtk::extension::qtItem;
using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

pqSMTKInfiniteCylinderItemWidget::pqSMTKInfiniteCylinderItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKInfiniteCylinderItemWidget::~pqSMTKInfiniteCylinderItemWidget() = default;

qtItem* pqSMTKInfiniteCylinderItemWidget::createCylinderItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKInfiniteCylinderItemWidget(info);
}

bool pqSMTKInfiniteCylinderItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  ItemBindings binding;
  std::vector<smtk::attribute::DoubleItemPtr> items;
  bool haveItems = this->fetchCylinderItems(binding, items);
  if (!haveItems || binding == ItemBindings::Invalid)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not find items for widget.");
    return false;
  }

  // I. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "Cylinder", server, "");
  if (!proxy)
  {
    return false;
  }
  widget = new pqCylinderPropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // II. Initialize the properties.
  m_p->m_pvwidget = widget;
  this->updateWidgetFromItem();
  auto* widgetProxy = widget->widgetProxy();
  widgetProxy->UpdateVTKObjects();
  // vtkSMPropertyHelper(widgetProxy, "RotationEnabled").Set(false);

  return widget != nullptr;
}

void pqSMTKInfiniteCylinderItemWidget::updateItemFromWidgetInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  ItemBindings binding;
  if (!this->fetchCylinderItems(binding, items))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item widget has an update but the item(s) do not exist or are not sized properly.");
    return;
  }

  // Values held by widget
  vtkVector3d center;
  vtkVector3d axis;
  double radius;
  vtkSMPropertyHelper centerHelper(widget, "Center");
  vtkSMPropertyHelper radiusHelper(widget, "Radius");
  vtkSMPropertyHelper axisHelper(widget, "Axis");
  centerHelper.Get(center.GetData(), 3);
  axisHelper.Get(axis.GetData(), 3);
  radiusHelper.Get(&radius, 1);
  bool didChange = false;

  // Current values held in items:
  vtkVector3d curAxis;
  vtkVector3d curCenter;
  double curRadius;

  // Translate widget values to item values and fetch current item values:
  if (binding == ItemBindings::CenterHalfAxisRadius)
  {
    curCenter = vtkVector3d(&(*items[0]->begin()));
    curAxis = vtkVector3d(&(*items[1]->begin()));
    curRadius = *items[2]->begin();
  }
  switch (binding)
  {
    case ItemBindings::CenterHalfAxisRadius:
      if (curCenter != center || curAxis != axis || curRadius != radius)
      {
        didChange = true;
        items[0]->setValues(center.GetData(), center.GetData() + 3);
        items[1]->setValues(axis.GetData(), axis.GetData() + 3);
        items[2]->setValue(radius);
      }
      break;
    case ItemBindings::Invalid:
    default:
      smtkErrorMacro(smtk::io::Logger::instance(), "Grrk");
      break;
  }

  if (didChange)
  {
    Q_EMIT modified();
  }
}

void pqSMTKInfiniteCylinderItemWidget::updateWidgetFromItemInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  ItemBindings binding;
  if (!this->fetchCylinderItems(binding, items))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item signaled an update but the item(s) do not exist or are not sized properly.");
    return;
  }

  // Unlike updateItemFromWidget, we don't care if we cause ParaView an unnecessary update;
  // we might cause an extra render but we won't accidentally mark a resource as modified.
  // Since there's no need to compare new values to old, this is simpler than updateItemFromWidget:
  switch (binding)
  {
    case ItemBindings::CenterHalfAxisRadius:
    {
      vtkVector3d center(&(*items[0]->begin()));
      vtkVector3d axis(&(*items[1]->begin()));
      double radius = items[2]->value(0);

      vtkSMPropertyHelper(widget, "Center").Set(center.GetData(), 3);
      vtkSMPropertyHelper(widget, "Axis").Set(axis.GetData(), 3);
      vtkSMPropertyHelper(widget, "Radius").Set(&radius, 1);
    }
    break;
    case ItemBindings::Invalid:
    default:
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Unable to determine item binding.");
    }
    break;
  }
}

bool pqSMTKInfiniteCylinderItemWidget::fetchCylinderItems(
  ItemBindings& binding,
  std::vector<smtk::attribute::DoubleItemPtr>& items)
{
  items.clear();

  // Check to see if item is a group containing items of double-vector items.
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 3)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Expected a group item with 1 group of 3 or more items.");
    return false;
  }

  // Find items in the group based on names in the configuration info:
  // ctr, axs, rad
  std::string ctrItemName;
  std::string axsItemName;
  std::string radItemName;
  if (!m_itemInfo.component().attribute("Center", ctrItemName))
  {
    ctrItemName = "Center";
  }
  if (!m_itemInfo.component().attribute("Axis", axsItemName))
  {
    axsItemName = "Axis";
  }
  if (!m_itemInfo.component().attribute("Radius", radItemName))
  {
    radItemName = "Radius";
  }
  auto ctrItem = groupItem->findAs<smtk::attribute::DoubleItem>(ctrItemName);
  auto axsItem = groupItem->findAs<smtk::attribute::DoubleItem>(axsItemName);
  auto radItem = groupItem->findAs<smtk::attribute::DoubleItem>(radItemName);

  if (
    ctrItem && ctrItem->numberOfValues() == 3 && axsItem && axsItem->numberOfValues() == 3 &&
    radItem && radItem->numberOfValues() == 1)
  {
    items.push_back(ctrItem);
    items.push_back(axsItem);
    items.push_back(radItem);
    binding = ItemBindings::CenterHalfAxisRadius;
    return true;
  }

  binding = ItemBindings::Invalid;
  return false;
}

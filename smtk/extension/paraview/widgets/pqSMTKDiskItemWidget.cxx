//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKDiskItemWidget.h"
#include "smtk/extension/paraview/widgets/pqDiskPropertyWidget.h"
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

pqSMTKDiskItemWidget::pqSMTKDiskItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKDiskItemWidget::~pqSMTKDiskItemWidget() = default;

qtItem* pqSMTKDiskItemWidget::createDiskItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKDiskItemWidget(info);
}

bool pqSMTKDiskItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  ItemBindings binding;
  std::vector<smtk::attribute::DoubleItemPtr> items;
  bool haveItems = this->fetchDiskItems(binding, items);
  if (!haveItems || binding == ItemBindings::Invalid)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not find items for widget.");
    return false;
  }

  // I. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "ImplicitDisk", server, "");
  if (!proxy)
  {
    return false;
  }
  auto* diskWidget = new pqDiskPropertyWidget(proxy, proxy->GetPropertyGroup(0));
  widget = diskWidget;

  // II. Initialize the properties.
  m_p->m_pvwidget = widget;
  this->updateWidgetFromItem();
  auto* widgetProxy = widget->widgetProxy();
  widgetProxy->UpdateVTKObjects();
  // vtkSMPropertyHelper(widgetProxy, "RotationEnabled").Set(false);

  return widget != nullptr;
}

bool pqSMTKDiskItemWidget::updateItemFromWidgetInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  ItemBindings binding;
  if (!this->fetchDiskItems(binding, items))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item widget has an update but the item(s) do not exist or are not sized properly.");
    return false;
  }

  // Values held by widget
  vtkVector3d ctr;
  vtkVector3d nrm;
  double rad;
  vtkSMPropertyHelper ctrHelper(widget, "CenterPoint");
  vtkSMPropertyHelper nrmHelper(widget, "Normal");
  vtkSMPropertyHelper radHelper(widget, "Radius");
  ctrHelper.Get(ctr.GetData(), 3);
  nrmHelper.Get(nrm.GetData(), 3);
  radHelper.Get(&rad, 1);
  bool didChange = false;

  // Current values held in items:
  vtkVector3d curPt0;
  vtkVector3d curPt1;
  double curRad;

  // Translate widget values to item values and fetch current item values:
  curPt0 = vtkVector3d(&(*items[0]->begin()));
  curPt1 = vtkVector3d(&(*items[1]->begin()));
  curRad = *items[2]->begin();
  switch (binding)
  {
    case ItemBindings::DiskPointsRadii:
      if (curPt0 != ctr || curPt1 != nrm || curRad != rad)
      {
        didChange = true;
        items[0]->setValues(ctr.GetData(), ctr.GetData() + 3);
        items[1]->setValues(nrm.GetData(), nrm.GetData() + 3);
        items[2]->setValue(rad);
      }
      break;
    case ItemBindings::Invalid:
    default:
      smtkErrorMacro(smtk::io::Logger::instance(), "Unable to determine item binding.");
      break;
  }

  return didChange;
}

bool pqSMTKDiskItemWidget::updateWidgetFromItemInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  ItemBindings binding;
  if (!this->fetchDiskItems(binding, items))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item signaled an update but the item(s) do not exist or are not sized properly.");
    return false;
  }

  // Unlike updateItemFromWidget, we don't care if we cause ParaView an unnecessary update;
  // we might cause an extra render but we won't accidentally mark a resource as modified.
  // Since there's no need to compare new values to old, this is simpler than updateItemFromWidget:
  vtkVector3d ctr(&(*items[0]->begin()));
  vtkVector3d nrm(&(*items[1]->begin()));
  double radius = items[2]->value(0);
  vtkSMPropertyHelper(widget, "CenterPoint").Set(ctr.GetData(), 3);
  vtkSMPropertyHelper(widget, "Normal").Set(nrm.GetData(), 3);
  vtkSMPropertyHelper(widget, "Radius").Set(&radius, 1);
  switch (binding)
  {
    case ItemBindings::DiskPointsRadii:
      break;
    case ItemBindings::Invalid:
    default:
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Unhandled item binding.");
    }
    break;
  }
  return true; // TODO: determine whether values were changed to avoid unnecessary renders.
}

bool pqSMTKDiskItemWidget::fetchDiskItems(
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
  // ctr, nrm, rad
  std::string ctrItemName;
  std::string nrmItemName;
  std::string radItemName;
  if (!m_itemInfo.component().attribute("Center", ctrItemName))
  {
    ctrItemName = "Center";
  }
  if (!m_itemInfo.component().attribute("Normal", nrmItemName))
  {
    nrmItemName = "Normal";
  }
  if (!m_itemInfo.component().attribute("Radius", radItemName))
  {
    radItemName = "Radius";
  }
  auto ctrItem = groupItem->findAs<smtk::attribute::DoubleItem>(ctrItemName);
  auto nrmItem = groupItem->findAs<smtk::attribute::DoubleItem>(nrmItemName);
  auto radItem = groupItem->findAs<smtk::attribute::DoubleItem>(radItemName);

  if (
    ctrItem && ctrItem->numberOfValues() == 3 && nrmItem && nrmItem->numberOfValues() == 3 &&
    radItem && radItem->numberOfValues() == 1)
  {
    items.push_back(ctrItem);
    items.push_back(nrmItem);
    items.push_back(radItem);
    binding = ItemBindings::DiskPointsRadii;
    return true;
  }

  binding = ItemBindings::Invalid;
  return false;
}

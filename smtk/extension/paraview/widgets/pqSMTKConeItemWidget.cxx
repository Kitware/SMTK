//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKConeItemWidget.h"
#include "smtk/extension/paraview/widgets/pqConePropertyWidget.h"
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

pqSMTKConeItemWidget::pqSMTKConeItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
  , m_forceCylinder(false)
{
  this->createWidget();
}

pqSMTKConeItemWidget::~pqSMTKConeItemWidget() = default;

qtItem* pqSMTKConeItemWidget::createConeItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKConeItemWidget(info);
}

qtItem* pqSMTKConeItemWidget::createCylinderItemWidget(const qtAttributeItemInfo& info)
{
  auto* item = new pqSMTKConeItemWidget(info);
  item->setForceCylindrical(true);
  return item;
}

bool pqSMTKConeItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  ItemBindings binding;
  std::vector<smtk::attribute::DoubleItemPtr> items;
  bool haveItems = this->fetchConeItems(binding, items);
  if (!haveItems || binding == ItemBindings::Invalid)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not find items for widget.");
    return false;
  }

  // I. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "ImplicitConeFrustum", server, "");
  if (!proxy)
  {
    return false;
  }
  auto* coneWidget = new pqConePropertyWidget(proxy, proxy->GetPropertyGroup(0));
  coneWidget->setForceCylindrical(m_forceCylinder);
  widget = coneWidget;

  // II. Initialize the properties.
  m_p->m_pvwidget = widget;
  this->updateWidgetFromItem();
  auto* widgetProxy = widget->widgetProxy();
  widgetProxy->UpdateVTKObjects();
  // vtkSMPropertyHelper(widgetProxy, "RotationEnabled").Set(false);

  return widget != nullptr;
}

bool pqSMTKConeItemWidget::updateItemFromWidgetInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  ItemBindings binding;
  if (!this->fetchConeItems(binding, items))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item widget has an update but the item(s) do not exist or are not sized properly.");
    return false;
  }

  // Values held by widget
  vtkVector3d pt0;
  vtkVector3d pt1;
  double rd0;
  double rd1;
  vtkSMPropertyHelper pt0Helper(widget, "BottomPoint");
  vtkSMPropertyHelper rd0Helper(widget, "BottomRadius");
  vtkSMPropertyHelper pt1Helper(widget, "TopPoint");
  pt0Helper.Get(pt0.GetData(), 3);
  pt1Helper.Get(pt1.GetData(), 3);
  rd0Helper.Get(&rd0, 1);
  bool didChange = false;

  // Current values held in items:
  vtkVector3d curPt0;
  vtkVector3d curPt1;
  double curRd0;
  double curRd1;

  // Translate widget values to item values and fetch current item values:
  curPt0 = vtkVector3d(&(*items[0]->begin()));
  curPt1 = vtkVector3d(&(*items[1]->begin()));
  curRd0 = *items[2]->begin();
  if (binding == ItemBindings::ConePointsRadii)
  {
    vtkSMPropertyHelper rd1Helper(widget, "TopRadius");
    rd0Helper.Get(&rd1, 1);
    curRd1 = *items[3]->begin();
  }
  else
  {
    rd1 = rd0;
    curRd1 = curRd0;
  }
  switch (binding)
  {
    case ItemBindings::CylinderPointsRadius:
      if (curPt0 != pt0 || curPt1 != pt1 || curRd0 != rd0)
      {
        didChange = true;
        items[0]->setValues(pt0.GetData(), pt0.GetData() + 3);
        items[1]->setValues(pt1.GetData(), pt1.GetData() + 3);
        items[2]->setValue(rd0);
      }
      break;
    case ItemBindings::ConePointsRadii:
      if (curPt0 != pt0 || curPt1 != pt1 || curRd0 != rd0 || curRd1 != rd1)
      {
        didChange = true;
        items[0]->setValues(pt0.GetData(), pt0.GetData() + 3);
        items[1]->setValues(pt1.GetData(), pt1.GetData() + 3);
        items[2]->setValue(rd0);
        items[3]->setValue(rd1);
      }
      break;
    case ItemBindings::Invalid:
    default:
      smtkErrorMacro(smtk::io::Logger::instance(), "Unable to determine item binding.");
      break;
  }

  return didChange;
}

bool pqSMTKConeItemWidget::updateWidgetFromItemInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  ItemBindings binding;
  if (!this->fetchConeItems(binding, items))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item signaled an update but the item(s) do not exist or are not sized properly.");
    return false;
  }

  // Unlike updateItemFromWidget, we don't care if we cause ParaView an unnecessary update;
  // we might cause an extra render but we won't accidentally mark a resource as modified.
  // Since there's no need to compare new values to old, this is simpler than updateItemFromWidget:
  vtkVector3d pt0(&(*items[0]->begin()));
  vtkVector3d pt1(&(*items[1]->begin()));
  double radius = items[2]->value(0);
  vtkSMPropertyHelper(widget, "BottomPoint").Set(pt0.GetData(), 3);
  vtkSMPropertyHelper(widget, "TopPoint").Set(pt1.GetData(), 3);
  vtkSMPropertyHelper(widget, "BottomRadius").Set(&radius, 1);
  switch (binding)
  {
    case ItemBindings::ConePointsRadii:
    {
      double r1 = items[3]->value(0);
      vtkSMPropertyHelper(widget, "TopRadius").Set(&r1, 1);
    }
    break;
    case ItemBindings::CylinderPointsRadius:
      vtkSMPropertyHelper(widget, "TopRadius").Set(&radius, 1);
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

bool pqSMTKConeItemWidget::setForceCylindrical(bool isCylinder)
{
  if (m_forceCylinder == isCylinder)
  {
    return false;
  }

  m_forceCylinder = isCylinder;
  if (m_p->m_pvwidget)
  {
    auto* widget = reinterpret_cast<pqConePropertyWidget*>(m_p->m_pvwidget);
    widget->setForceCylindrical(m_forceCylinder);
  }
  return true;
}

bool pqSMTKConeItemWidget::fetchConeItems(
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
  // pt0, pt1, rd0
  std::string pt0ItemName;
  std::string pt1ItemName;
  std::string rd0ItemName;
  std::string rd1ItemName;
  if (!m_itemInfo.component().attribute("BottomPoint", pt0ItemName))
  {
    pt0ItemName = "BottomPoint";
  }
  if (!m_itemInfo.component().attribute("TopPoint", pt1ItemName))
  {
    pt1ItemName = "TopPoint";
  }
  if (!m_itemInfo.component().attribute("BottomRadius", rd0ItemName))
  {
    rd0ItemName = "BottomRadius";
  }
  if (!m_itemInfo.component().attribute("TopRadius", rd1ItemName))
  {
    rd1ItemName = "TopRadius";
  }
  // For cylinders, use the same attribute item for both radii.
  if (m_itemInfo.component().attribute("Radius", rd0ItemName))
  {
    rd1ItemName = rd0ItemName;
  }
  auto pt0Item = groupItem->findAs<smtk::attribute::DoubleItem>(pt0ItemName);
  auto pt1Item = groupItem->findAs<smtk::attribute::DoubleItem>(pt1ItemName);
  auto rd0Item = groupItem->findAs<smtk::attribute::DoubleItem>(rd0ItemName);
  auto rd1Item = groupItem->findAs<smtk::attribute::DoubleItem>(rd1ItemName);

  if (
    pt0Item && pt0Item->numberOfValues() == 3 && pt1Item && pt1Item->numberOfValues() == 3 &&
    rd0Item && rd0Item->numberOfValues() == 1)
  {
    items.push_back(pt0Item);
    items.push_back(pt1Item);
    items.push_back(rd0Item);
    if (!rd1Item || rd1Item == rd0Item)
    {
      binding = ItemBindings::CylinderPointsRadius;
      return true;
    }
    else if (rd1Item->numberOfValues() == 1)
    {
      items.push_back(rd1Item);
      binding = ItemBindings::ConePointsRadii;
      return true;
    }
  }

  binding = ItemBindings::Invalid;
  return false;
}

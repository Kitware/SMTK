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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqImplicitPlanePropertyWidget.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqPointPropertyWidget.h"
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

pqSMTKPointItemWidget::pqSMTKPointItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKPointItemWidget::~pqSMTKPointItemWidget() = default;

qtItem* pqSMTKPointItemWidget::createPointItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKPointItemWidget(info);
}

bool pqSMTKPointItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  // I. Reject items we can't map to a point:
  ItemBindings binding;
  smtk::attribute::DoubleItemPtr pointItem;
  smtk::attribute::StringItemPtr controlItem;
  if (!fetchPointItems(binding, pointItem, controlItem))
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
  auto* ww = new pqPointPropertyWidget(proxy, proxy->GetPropertyGroup(0));
  bool showControls = m_itemInfo.component().attributeAsBool("ShowControls");
  ww->setControlVisibility(showControls);
  if (binding == ItemBindings::PointCoordsAndControl)
  {
    ww->setControlState(controlItem->value());
    this->connect(
      ww, SIGNAL(controlStateChanged(const std::string&)), SLOT(updateItemFromWidget()));
  }
  widget = ww;

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  auto* widgetProxy = widget->widgetProxy();
  vtkSMPropertyHelper(widgetProxy, "WorldPosition").Set(&(*pointItem->begin()), 3);

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKPointItemWidget::updateItemFromWidgetInternal()
{
  ItemBindings binding;
  smtk::attribute::DoubleItemPtr pointItem;
  smtk::attribute::StringItemPtr controlItem;
  if (!fetchPointItems(binding, pointItem, controlItem))
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
  if (binding == ItemBindings::PointCoordsAndControl)
  {
    auto* ww = dynamic_cast<pqPointPropertyWidget*>(this->propertyWidget());
    if (ww)
    {
      int oldIndex = controlItem->discreteIndex();
      if (controlItem->setValue(ww->controlState()))
      {
        int newIndex = controlItem->discreteIndex();
        didChange = (newIndex != oldIndex);
      }
    }
  }

  if (didChange)
  {
    Q_EMIT modified();
  }
}

void pqSMTKPointItemWidget::updateWidgetFromItemInternal()
{
  ItemBindings binding;
  smtk::attribute::DoubleItemPtr pointItem;
  smtk::attribute::StringItemPtr controlItem;
  if (!fetchPointItems(binding, pointItem, controlItem))
  {
    return;
  }

  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  auto* pw = dynamic_cast<pqPointPropertyWidget*>(m_p->m_pvwidget);
  vtkSMPropertyHelper pointHelper(widget, "WorldPosition");
  pointHelper.Set(&(*pointItem->begin()), 3);
  if (binding == ItemBindings::PointCoordsAndControl && pw)
  {
    pw->setControlState(controlItem->value());
  }
}

bool pqSMTKPointItemWidget::fetchPointItems(
  ItemBindings& binding,
  smtk::attribute::DoubleItemPtr& coordItem,
  smtk::attribute::StringItemPtr& controlItem)
{
  coordItem = m_itemInfo.itemAs<smtk::attribute::DoubleItem>();
  if (coordItem && coordItem->numberOfValues() == 3)
  {
    std::string controlItemPath;
    m_itemInfo.component().attribute("Control", controlItemPath);
    controlItem =
      coordItem->attribute()->itemAtPathAs<smtk::attribute::StringItem>(controlItemPath);
    if (this->validateControlItem(controlItem))
    {
      binding = ItemBindings::PointCoordsAndControl;
    }
    else
    {
      controlItem = nullptr;
      binding = ItemBindings::PointCoords;
    }
    return true;
  }

  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (groupItem)
  {
    std::string coordItemName;
    std::string controlItemName;
    m_itemInfo.component().attribute("Coords", coordItemName);
    m_itemInfo.component().attribute("Control", controlItemName);
    coordItem = groupItem->findAs<smtk::attribute::DoubleItem>(0, coordItemName);
    controlItem = groupItem->findAs<smtk::attribute::StringItem>(0, controlItemName);
    // We must have coordinates as specified:
    if (coordItem && coordItem->numberOfValues() == 3)
    {
      // TODO: Check discrete enumerants?
      if (controlItem && controlItem->isDiscrete())
      {
        binding = ItemBindings::PointCoordsAndControl;
      }
      else
      {
        binding = ItemBindings::PointCoords;
      }
      return true;
    }
  }

  binding = ItemBindings::Invalid;
  smtkErrorMacro(smtk::io::Logger::instance(), "Expected a double item with 3 values.");
  return false;
}

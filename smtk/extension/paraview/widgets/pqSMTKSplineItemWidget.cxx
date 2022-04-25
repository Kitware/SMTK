//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKSplineItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqImplicitPlanePropertyWidget.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "pqSpherePropertyWidget.h"
#include "pqSplinePropertyWidget.h"
#include "vtkPVXMLElement.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

#include "vtkEventQtSlotConnect.h"

using qtItem = smtk::extension::qtItem;
using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

pqSMTKSplineItemWidget::pqSMTKSplineItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
  , m_handleConnection(vtkEventQtSlotConnect::New())
{
  this->createWidget();
}

pqSMTKSplineItemWidget::~pqSMTKSplineItemWidget()
{
  m_handleConnection->Delete();
  m_handleConnection = nullptr;
}

qtItem* pqSMTKSplineItemWidget::createSplineItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKSplineItemWidget(info);
}

bool pqSMTKSplineItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  // I. Reject items we can't map to our widget:
  smtk::attribute::DoubleItemPtr pointsItem;
  smtk::attribute::VoidItemPtr closedItem;
  if (!fetchPointsAndClosedItems(pointsItem, closedItem))
  {
    return false;
  }

  // Get rid of old connections; we're about to have a new object to watch.
  m_handleConnection->Disconnect();

  // II. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  pqSplinePropertyWidget::ModeTypes mode = m_itemInfo.component().attributeAsBool("Polyline")
    ? pqSplinePropertyWidget::POLYLINE
    : pqSplinePropertyWidget::SPLINE;
  if (mode == pqSplinePropertyWidget::POLYLINE)
  {
    proxy = builder->createProxy("extended_sources", "PolyLineSource", server, "");
  }
  else
  {
    proxy = builder->createProxy("parametric_functions", "Spline", server, "");
  }
  if (!proxy)
  {
    return false;
  }
  auto* splineWidget = new pqSplinePropertyWidget(proxy, proxy->GetPropertyGroup(0), mode);
  widget = splineWidget;
  std::string colorStr;
  if (m_itemInfo.component().attribute("Color", colorStr))
  {
    std::istringstream cs(colorStr);
    double rgb[3];
    for (int ii = 0; ii < 3; ++ii)
    {
      cs >> rgb[ii];
      // Clamp to [0,1]:
      rgb[ii] = (rgb[ii] < 0. ? 0. : (rgb[ii] > 1. ? 1. : rgb[ii]));
    }
    splineWidget->setLineColor(QColor::fromRgbF(rgb[0], rgb[1], rgb[2]));
  }

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  auto* widgetProxy = widget->widgetProxy();
  auto numberOfValues = static_cast<unsigned int>(pointsItem->numberOfValues());
  vtkSMPropertyHelper(widgetProxy, "HandlePositions").Set(&(*pointsItem->begin()), numberOfValues);
  vtkSMPropertyHelper(widgetProxy, "Closed").Set(closedItem->isEnabled());

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  m_handleConnection->Connect(
    widgetProxy->GetProperty("HandlePositions"),
    vtkCommand::ModifiedEvent,
    this,
    SIGNAL(modified()));

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKSplineItemWidget::updateItemFromWidgetInternal()
{
  smtk::attribute::DoubleItemPtr pointsItem;
  smtk::attribute::VoidItemPtr closedItem;
  if (!fetchPointsAndClosedItems(pointsItem, closedItem))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not update item from changes via widget.");
    return;
  }

  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();

  bool didChange = false;
  vtkSMPropertyHelper pointsHelper(widget, "HandlePositions");
  auto pointsArray = pointsHelper.GetArray<double>();
  if (!pointsItem->setValues(pointsArray.begin(), pointsArray.end()))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not update \"" << pointsItem->label()
                            << "\""
                               " with "
                            << pointsArray.size() << " coordinates.");
  }
  for (size_t ii = 0; ii < pointsItem->numberOfValues(); ++ii)
  {
    didChange |= (pointsItem->value(ii) != pointsArray[ii]);
  }

  vtkSMPropertyHelper closedHelper(widget, "Closed");
  bool closed = !!closedHelper.GetAsInt();
  didChange |= (closedItem->isEnabled() != closed);
  closedItem->setIsEnabled(closed);

  if (didChange)
  {
    Q_EMIT modified();
  }
}

bool pqSMTKSplineItemWidget::fetchPointsAndClosedItems(
  smtk::attribute::DoubleItemPtr& pointsItem,
  smtk::attribute::VoidItemPtr& closedItem)
{
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 2)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Expected a group item with 1 group of 2 items.");
    return false;
  }
  std::string pointsItemName;
  std::string closedItemName;
  if (!m_itemInfo.component().attribute("Points", pointsItemName))
  {
    pointsItemName = "Points";
  }
  if (!m_itemInfo.component().attribute("Closed", closedItemName))
  {
    closedItemName = "Closed";
  }
  pointsItem = groupItem->findAs<smtk::attribute::DoubleItem>(pointsItemName);
  closedItem = groupItem->findAs<smtk::attribute::VoidItem>(closedItemName);
  if (!pointsItem || !closedItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find"
        << " a DoubleItem for the points named \"" << pointsItemName << "\","
        << " a VoidItem for loop-closure named \"" << closedItemName << "\","
        << " or both.");
    return false;
  }
  if (pointsItem->numberOfValues() % 3 != 0)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "The points (" << pointsItem->numberOfValues() << ") must have"
                     << " values in multiples of 3 (x, y, and z for each point).");
    return false;
  }
  if (closedItem->isEnabled() && (pointsItem->numberOfValues() / 3 < 3))
  {
    // TODO: Handle splines differently? Does VTK allow 2-point splines?
    smtkErrorMacro(smtk::io::Logger::instance(), "Closed curves must have at least 3 points.");
    return false;
  }
  return true;
}

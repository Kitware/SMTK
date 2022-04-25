//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKBoxItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqBoxPropertyWidget.h"
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

#include <QCheckBox>

#include <algorithm>
#include <cctype>

using qtItem = smtk::extension::qtItem;
using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

pqSMTKBoxItemWidget::pqSMTKBoxItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKBoxItemWidget::~pqSMTKBoxItemWidget() = default;

qtItem* pqSMTKBoxItemWidget::createBoxItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKBoxItemWidget(info);
}

bool pqSMTKBoxItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  ItemBindings binding;
  std::vector<smtk::attribute::DoubleItemPtr> items;
  smtk::attribute::StringItemPtr control;
  bool haveItems = this->fetchBoxItems(binding, items, control);
  if (!haveItems || binding == ItemBindings::Invalid)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not find items for widget.");
    return false;
  }

  // I. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  // The paraview widget uses the list of available properties in the
  // proxy's property group to decide which UI elements should be
  // exposed. Specifically, rotation is disabled if there is no
  // property for the Euler angles. So... choose which object the proxy
  // should point to based on whether the SMTK items model Euler angles
  // or not:
  if (binding <= ItemBindings::AxisAlignedCenterDeltas)
  {
    proxy = builder->createProxy("smtk_widgets", "SMTKAxisAlignedBoxWidget", server, "");
  }
  else
  {
    proxy = builder->createProxy("implicit_functions", "Box", server, "");
  }
  if (!proxy)
  {
    return false;
  }
  widget = new pqBoxPropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // Unlike traditional boolean-valued XML attributes, ShowControls defaults
  // to true (even when not present) to preserve existing behavior.
  bool showControls;
  bool haveShowControls = m_itemInfo.component().attributeAsBool("ShowControls", showControls);
  showControls |= !haveShowControls; // when ShowControls not present, showControls should be true

  auto* visibility = widget->findChild<QCheckBox*>("show3DWidget");
  if (showControls)
  {
    visibility->show();
  }
  else
  {
    visibility->hide();
  }

  if (control)
  {
    this->setControlState(control->value(), visibility);
    this->connect(visibility, SIGNAL(toggled(bool)), SLOT(updateItemFromWidget()));
  }

  // II. Initialize the properties.
  m_p->m_pvwidget = widget;
  this->updateWidgetFromItem();
  auto* widgetProxy = widget->widgetProxy();
  widgetProxy->UpdateVTKObjects();
  // vtkSMPropertyHelper(widgetProxy, "RotationEnabled").Set(false);

  return widget != nullptr;
}

void pqSMTKBoxItemWidget::updateItemFromWidgetInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  smtk::attribute::StringItemPtr control;
  ItemBindings binding;
  if (!this->fetchBoxItems(binding, items, control))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item widget has an update but the item(s) do not exist or are not sized properly.");
    return;
  }

  bool didChange = false;

  if (control)
  {
    auto* visibility = this->propertyWidget()->findChild<QCheckBox*>("show3DWidget");
    std::string oldValue = control->value();
    switch (visibility->checkState())
    {
      case Qt::Unchecked:
        control->setValue("inactive");
        break;
      case Qt::PartiallyChecked:
        control->setValue("visible");
        break;
      case Qt::Checked:
        control->setValue("active");
        break;
      default:
        break;
    }
    didChange |= (oldValue != control->value());
  }

  // Values held by widget
  vtkVector3d origin;
  vtkVector3d length;
  vtkVector3d angles;
  vtkSMPropertyHelper originHelper(widget, "Position");
  vtkSMPropertyHelper lengthHelper(widget, "Scale");
  vtkSMPropertyHelper anglesHelper(widget, "Rotation", /* quiet */ true);
  originHelper.Get(origin.GetData(), 3);
  lengthHelper.Get(length.GetData(), 3);
  vtkVector3d center;
  vtkVector3d deltas;
  vtkVector3d point2;

  // Current values held in items:
  vtkVector3d curAngles;
  vtkVector3d curCenter;
  vtkVector3d curPoint1;
  vtkVector3d curPoint2;
  vtkVector3d curDeltas;

  // Translate widget values to item values and fetch current item values:
  if (binding >= ItemBindings::EulerAngleBounds)
  {
    vtkVector3d e1, e2, e3;
    anglesHelper.Get(angles.GetData(), 3);
    const double cth = cos(vtkMath::RadiansFromDegrees(angles[0]));
    const double cph = cos(vtkMath::RadiansFromDegrees(angles[1]));
    const double cps = cos(vtkMath::RadiansFromDegrees(angles[2]));

    const double sth = sin(vtkMath::RadiansFromDegrees(angles[0]));
    const double sph = sin(vtkMath::RadiansFromDegrees(angles[1]));
    const double sps = sin(vtkMath::RadiansFromDegrees(angles[2]));

    // From https://en.wikipedia.org/wiki/Euler_angles#Intrinsic_rotations :
    // VTK uses Tait-Bryan Y_1 X_2 Z_3 angles to store orientation;
    // This is the corresponding direction cosine matrix (DCM) for
    // theta = X, phi = Y, psi = Z:
    e1 = { cth * cps + sth * sph * sps, cps * sth * sph - cth * sps, cph * sth };
    e2 = { cph * sps, cph * cps, -sph };
    e3 = { cth * sph * sps - cps * sth, cth * cps * sph + sth * sps, cph * cth };
    if (binding == ItemBindings::EulerAngleCenterDeltas)
    {
      deltas = 0.5 * length;
      center = origin + deltas[0] * e1 + deltas[1] * e2 + deltas[2] * e3;

      curCenter = vtkVector3d(&(*items[0]->begin()));
      curDeltas = vtkVector3d(&(*items[1]->begin()));
      curAngles = vtkVector3d(&(*items[2]->begin()));
    }
    else
    {
      point2 = origin + length[0] * e1 + length[1] * e2 + length[2] * e3;

      if (binding == ItemBindings::EulerAngleMinMax)
      {
        curPoint1 = vtkVector3d(&(*items[0]->begin()));
        curPoint2 = vtkVector3d(&(*items[1]->begin()));
        curAngles = vtkVector3d(&(*items[2]->begin()));
      }
      else // binding == ItemBindings::EulerAngleBounds
      {
        curPoint1 = vtkVector3d(items[0]->value(0), items[0]->value(2), items[0]->value(4));
        curPoint2 = vtkVector3d(items[0]->value(1), items[0]->value(3), items[0]->value(5));
        curAngles = vtkVector3d(&(*items[1]->begin()));
      }
    }
  }
  else
  {
    if (binding == ItemBindings::AxisAlignedCenterDeltas)
    {
      deltas = 0.5 * length;
      center = origin + deltas;

      curCenter = vtkVector3d(&(*items[0]->begin()));
      curDeltas = vtkVector3d(&(*items[1]->begin()));
    }
    else
    {
      point2 = origin + length;

      if (binding == ItemBindings::AxisAlignedMinMax)
      {
        curPoint1 = vtkVector3d(&(*items[0]->begin()));
        curPoint2 = vtkVector3d(&(*items[1]->begin()));
      }
      else
      {
        curPoint1 = vtkVector3d(items[0]->value(0), items[0]->value(2), items[0]->value(4));
        curPoint2 = vtkVector3d(items[0]->value(1), items[0]->value(3), items[0]->value(5));
      }
    }
  }
  switch (binding)
  {
    case ItemBindings::AxisAlignedBounds:
    case ItemBindings::EulerAngleBounds:
      if (curPoint1 != origin || curPoint2 != point2)
      {
        didChange = true;
        for (int ii = 0; ii < 3; ++ii)
        {
          items[0]->setValue(2 * ii, origin[ii]);
          items[0]->setValue(2 * ii + 1, point2[ii]);
        }
      }
      if (binding == ItemBindings::EulerAngleBounds && curAngles != angles)
      {
        didChange = true;
        items[1]->setValues(angles.GetData(), angles.GetData() + 3);
      }
      break;
    case ItemBindings::AxisAlignedMinMax:
    case ItemBindings::EulerAngleMinMax:
      if (curPoint1 != origin || curPoint2 != point2)
      {
        didChange = true;
        items[0]->setValues(origin.GetData(), origin.GetData() + 3);
        items[1]->setValues(point2.GetData(), point2.GetData() + 3);
      }
      if (binding == ItemBindings::EulerAngleMinMax && curAngles != angles)
      {
        didChange = true;
        items[2]->setValues(angles.GetData(), angles.GetData() + 3);
      }
      break;
    case ItemBindings::AxisAlignedCenterDeltas:
    case ItemBindings::EulerAngleCenterDeltas:
      if (curCenter != center || curDeltas != deltas)
      {
        didChange = true;
        items[0]->setValues(center.GetData(), center.GetData() + 3);
        items[1]->setValues(deltas.GetData(), deltas.GetData() + 3);
      }
      if (binding == ItemBindings::EulerAngleCenterDeltas && curAngles != angles)
      {
        didChange = true;
        items[2]->setValues(angles.GetData(), angles.GetData() + 3);
      }
      break;
    case ItemBindings::Invalid:
    default:
      smtkErrorMacro(smtk::io::Logger::instance(), "Unable to determine item binding.");
      break;
  }

  if (didChange)
  {
    Q_EMIT modified();
  }
}

void pqSMTKBoxItemWidget::updateWidgetFromItemInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  smtk::attribute::StringItemPtr control;
  ItemBindings binding;
  if (!this->fetchBoxItems(binding, items, control))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item signaled an update but the item(s) do not exist or are not sized properly.");
    return;
  }

  if (control)
  {
    auto* pw = this->propertyWidget();
    auto* visibility = pw ? pw->findChild<QCheckBox*>("show3DWidget") : nullptr;
    if (visibility)
    {
      this->setControlState(control->value(), visibility);
    }
  }

  // Unlike updateItemFromWidget, we don't care if we cause ParaView an unnecessary update;
  // we might cause an extra render but we won't accidentally mark a resource as modified.
  // Since there's no need to compare new values to old, this is simpler than updateItemFromWidget:
  switch (binding)
  {
    case ItemBindings::AxisAlignedBounds:
    case ItemBindings::EulerAngleBounds:
    {
      vtkVector3d point1(items[0]->value(0), items[0]->value(2), items[0]->value(4));
      vtkVector3d point2(items[0]->value(1), items[0]->value(3), items[0]->value(5));
      vtkVector3d length = point2 - point1;

      vtkSMPropertyHelper(widget, "Position").Set(point1.GetData(), 3);
      vtkSMPropertyHelper(widget, "Scale").Set(length.GetData(), 3);
      if (binding == ItemBindings::EulerAngleBounds)
      {
        vtkVector3d angles(&(*items[1]->begin()));
        vtkSMPropertyHelper(widget, "Rotation").Set(angles.GetData(), 3);
      }
    }
    break;
    case ItemBindings::AxisAlignedMinMax:
    case ItemBindings::EulerAngleMinMax:
    {
      vtkVector3d point1(&(*items[0]->begin()));
      vtkVector3d point2(&(*items[1]->begin()));
      vtkVector3d length = point2 - point1;

      vtkSMPropertyHelper(widget, "Position").Set(point1.GetData(), 3);
      vtkSMPropertyHelper(widget, "Scale").Set(length.GetData(), 3);
      if (binding == ItemBindings::EulerAngleMinMax)
      {
        vtkVector3d angles(&(*items[2]->begin()));
        vtkSMPropertyHelper(widget, "Rotation").Set(angles.GetData(), 3);
      }
    }
    break;
    case ItemBindings::AxisAlignedCenterDeltas:
    case ItemBindings::EulerAngleCenterDeltas:
    {
      vtkVector3d center(&(*items[0]->begin()));
      vtkVector3d deltas(&(*items[1]->begin()));
      vtkVector3d origin;
      vtkVector3d length = 2 * deltas;
      if (binding == ItemBindings::AxisAlignedCenterDeltas)
      {
        origin = center - deltas;
      }
      else
      {
        vtkVector3d angles(&(*items[2]->begin()));
        vtkSMPropertyHelper(widget, "Rotation").Set(angles.GetData(), 3);

        const double cth = cos(vtkMath::RadiansFromDegrees(angles[0]));
        const double cph = cos(vtkMath::RadiansFromDegrees(angles[1]));
        const double cps = cos(vtkMath::RadiansFromDegrees(angles[2]));

        const double sth = sin(vtkMath::RadiansFromDegrees(angles[0]));
        const double sph = sin(vtkMath::RadiansFromDegrees(angles[1]));
        const double sps = sin(vtkMath::RadiansFromDegrees(angles[2]));

        // From https://en.wikipedia.org/wiki/Euler_angles#Intrinsic_rotations :
        // VTK uses Tait-Bryan Y_1 X_2 Z_3 angles to store orientation;
        // This is the corresponding direction cosine matrix (DCM) for
        // theta = X, phi = Y, psi = Z:
        vtkVector3d e1 = { cth * cps + sth * sph * sps, cps * sth * sph - cth * sps, cph * sth };
        vtkVector3d e2 = { cph * sps, cph * cps, -sph };
        vtkVector3d e3 = { cth * sph * sps - cps * sth, cth * cps * sph + sth * sps, cph * cth };
        origin = center - deltas[0] * e1 - deltas[1] * e2 - deltas[2] * e3;
      }
      vtkSMPropertyHelper(widget, "Position").Set(origin.GetData(), 3);
      vtkSMPropertyHelper(widget, "Scale").Set(length.GetData(), 3);
    }
    break;
    case ItemBindings::Invalid:
    default:
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Grrk");
    }
    break;
  }
}

bool pqSMTKBoxItemWidget::fetchBoxItems(
  ItemBindings& binding,
  std::vector<smtk::attribute::DoubleItemPtr>& items,
  smtk::attribute::StringItemPtr& control)
{
  items.clear();
  control = nullptr;

  // Check to see if item is a vector of doubles:
  auto doubleItem = m_itemInfo.itemAs<smtk::attribute::DoubleItem>();
  if (doubleItem)
  {
    if (doubleItem->numberOfValues() == 6)
    {
      binding = ItemBindings::AxisAlignedBounds;
      items.push_back(doubleItem);
      return true;
    }

    binding = ItemBindings::Invalid;
    return false;
  }

  // Check to see if item is a group containing items of double-vector items.
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 2)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Expected a group item with 1 group of 2 or more items.");
    return false;
  }

  std::string controlItemName;
  m_itemInfo.component().attribute("Control", controlItemName);
  control = groupItem->findAs<smtk::attribute::StringItem>(0, controlItemName);
  if (!this->validateControlItem(control))
  {
    // Do not accept an item without discrete values. TODO: Check discrete values.
    control = nullptr;
  }

  // Find items in the group based on names in the configuration info:
  // bds, min, max, center, delta, angle
  std::string bdsItemName;
  std::string minItemName;
  std::string maxItemName;
  std::string ctrItemName;
  std::string dltItemName;
  std::string angItemName;
  if (!m_itemInfo.component().attribute("Bounds", bdsItemName))
  {
    bdsItemName = "Bounds";
  }
  if (!m_itemInfo.component().attribute("Min", minItemName))
  {
    minItemName = "Min";
  }
  if (!m_itemInfo.component().attribute("Max", maxItemName))
  {
    maxItemName = "Max";
  }
  if (!m_itemInfo.component().attribute("Center", ctrItemName))
  {
    ctrItemName = "Center";
  }
  if (!m_itemInfo.component().attribute("Deltas", dltItemName))
  {
    dltItemName = "Deltas";
  }
  if (!m_itemInfo.component().attribute("Angles", angItemName))
  {
    angItemName = "Angles";
  }
  auto bdsItem = groupItem->findAs<smtk::attribute::DoubleItem>(bdsItemName);
  auto minItem = groupItem->findAs<smtk::attribute::DoubleItem>(minItemName);
  auto maxItem = groupItem->findAs<smtk::attribute::DoubleItem>(maxItemName);
  auto ctrItem = groupItem->findAs<smtk::attribute::DoubleItem>(ctrItemName);
  auto dltItem = groupItem->findAs<smtk::attribute::DoubleItem>(dltItemName);
  auto angItem = groupItem->findAs<smtk::attribute::DoubleItem>(angItemName);

  bool angleItemInUse =
    angItem && (!angItem->isOptional() || angItem->isEnabled()) && angItem->numberOfValues() == 3;

  if (bdsItem && bdsItem->numberOfValues() == 6)
  {
    items.push_back(bdsItem);
    if (angleItemInUse)
    {
      items.push_back(angItem);
      binding = ItemBindings::EulerAngleBounds;
      return true;
    }
    else
    {
      binding = ItemBindings::AxisAlignedBounds;
      return true;
    }
  }

  if (minItem && maxItem && minItem->numberOfValues() == 3 && maxItem->numberOfValues() == 3)
  {
    items.push_back(minItem);
    items.push_back(maxItem);
    if (angleItemInUse)
    {
      items.push_back(angItem);
      binding = ItemBindings::EulerAngleMinMax;
      return true;
    }
    else
    {
      binding = ItemBindings::AxisAlignedMinMax;
      return true;
    }
  }

  if (ctrItem && dltItem && ctrItem->numberOfValues() == 3 && dltItem->numberOfValues() == 3)
  {
    items.push_back(ctrItem);
    items.push_back(dltItem);
    if (angleItemInUse)
    {
      items.push_back(angItem);
      binding = ItemBindings::EulerAngleCenterDeltas;
      return true;
    }
    else
    {
      binding = ItemBindings::AxisAlignedCenterDeltas;
      return true;
    }
  }

  binding = ItemBindings::Invalid;
  return false;
}

void pqSMTKBoxItemWidget::setControlState(const std::string& controlState, QCheckBox* controlWidget)
{
  std::string state = controlState;
  std::transform(
    state.begin(), state.end(), state.begin(), [](unsigned char c) { return std::tolower(c); });
  if (state == "active")
  {
    controlWidget->setCheckState(Qt::Checked);
  }
  else if (state == "visible")
  {
    controlWidget->setCheckState(Qt::PartiallyChecked);
  }
  else if (state == "inactive")
  {
    controlWidget->setCheckState(Qt::Unchecked);
  }
}

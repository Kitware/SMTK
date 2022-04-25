//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKSliceItemWidget.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/utility/Queries.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCrinklePropertyWidget.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "pqSlicePropertyWidget.h"
#include "pqSpherePropertyWidget.h"
#include "vtkPVXMLElement.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

using qtItem = smtk::extension::qtItem;
using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

pqSMTKSliceItemWidget::pqSMTKSliceItemWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKSliceItemWidget::~pqSMTKSliceItemWidget() = default;

qtItem* pqSMTKSliceItemWidget::createSliceItemWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKSliceItemWidget(info);
}

bool pqSMTKSliceItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  //I. Reject items we can't map to a plane:
  smtk::attribute::DoubleItemPtr originItem;
  smtk::attribute::DoubleItemPtr normalItem;
  smtk::attribute::ReferenceItemPtr inputsItem;
  if (!fetchOriginNormalAndInputsItems(originItem, normalItem, inputsItem))
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

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  std::shared_ptr<smtk::resource::Resource> resource;
  {
    auto resourceSet = smtk::attribute::utility::extractResources(inputsItem);
    if (resourceSet.empty())
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "Associate at least one component.");
    }
    else
    {
      auto it = resourceSet.begin();
      resource = *it;
      ++it;
      if (it != resourceSet.end())
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(),
          "Associate component(s) from exactly a single resource. Ignoring all but one resource.");
      }
    }
  }
  auto* behavior = pqSMTKBehavior::instance();
  auto pipeline = behavior->getPVResource(resource);
  std::string sliceStyle("Flat");
  m_itemInfo.component().attribute("Style", sliceStyle);
  if (sliceStyle == "Crinkle")
  {
    widget = new pqCrinklePropertyWidget(pipeline, proxy, proxy->GetPropertyGroup(0));
  }
  else
  {
    widget = new pqSlicePropertyWidget(pipeline, proxy, proxy->GetPropertyGroup(0));
  }
  auto* widgetProxy = widget->widgetProxy();
  if (pipeline)
  {
    vtkSMPropertyHelper(widgetProxy, "Input").Set(pipeline->getProxy());
  }
  vtkSMPropertyHelper(widgetProxy, "Origin").Set(&(*originItem->begin()), 3);
  vtkSMPropertyHelper(widgetProxy, "Normal").Set(&(*normalItem->begin()), 3);
  int drawOutline = 0;
  vtkSMPropertyHelper(widgetProxy, "DrawOutline").Set(drawOutline, 0);

  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  widgetProxy->UpdateVTKObjects();

  // This assignment is also done by caller, but needed for the sliceInputsChanged() below,
  // which initializes the set of blocks to extract and apply the crinkle-filter to.
  m_p->m_pvwidget = widget;
  this->sliceInputsChanged();

  return widget != nullptr;
}

void pqSMTKSliceItemWidget::sliceInputsChanged()
{
  smtk::attribute::DoubleItemPtr originItem;
  smtk::attribute::DoubleItemPtr normalItem;
  smtk::attribute::ReferenceItemPtr inputsItem;
  if (this->fetchOriginNormalAndInputsItems(originItem, normalItem, inputsItem))
  {
    vtkSMNewWidgetRepresentationProxy* widgetProxy = m_p->m_pvwidget->widgetProxy();
    // First, if we haven't set the input proxy (or we have but the first resource
    // has changed), update the widget's input.
    auto resourceSet = smtk::attribute::utility::extractResources(inputsItem);
    if (!resourceSet.empty())
    {
      auto* behavior = pqSMTKBehavior::instance();
      auto pipeline = behavior->getPVResource(*resourceSet.begin());
      if (pipeline)
      {
        vtkSMPropertyHelper(widgetProxy, "Input").Set(pipeline->getProxy());
        // We must immediately update the pipeline or the IDs will not get
        // set properly below.
        widgetProxy->UpdateVTKObjects();
      }
    }
    // Now update the component IDs.
    vtkSMPropertyHelper idsHelper(widgetProxy, "Ids");
    idsHelper.RemoveAllValues();
    for (std::size_t ii = 0; ii < inputsItem->numberOfValues(); ++ii)
    {
      if (inputsItem->isSet(ii))
      {
        // Copy the entry's UUID into a 4-integer tuple (this is
        // a factor of 2 too large on 64-bit platforms but ParaView
        // doesn't have server-manager items for fixed-size integers).
        smtk::common::UUID uid = inputsItem->value(ii)->id();
        std::array<int, 4> data{ { 0, 0, 0, 0 } };
        uint8_t* raw = reinterpret_cast<uint8_t*>(data.data());
        for (int jj = 0; jj < smtk::common::UUID::SIZE; ++jj)
        {
          raw[jj] = *(uid.begin() + jj);
        }
        idsHelper.Append(data.data(), 4);
      }
    }
    widgetProxy->UpdateVTKObjects();
    this->renderViewEventually();
  }
}

void pqSMTKSliceItemWidget::updateItemFromWidgetInternal()
{
  smtk::attribute::DoubleItemPtr originItem;
  smtk::attribute::DoubleItemPtr normalItem;
  smtk::attribute::ReferenceItemPtr inputsItem;
  if (!fetchOriginNormalAndInputsItems(originItem, normalItem, inputsItem))
  {
    return;
  }
  vtkSMNewWidgetRepresentationProxy* widgetProxy = m_p->m_pvwidget->widgetProxy();
  vtkSMPropertyHelper originHelper(widgetProxy, "Origin");
  vtkSMPropertyHelper normalHelper(widgetProxy, "Normal");
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

void pqSMTKSliceItemWidget::updateWidgetFromItemInternal()
{
  this->sliceInputsChanged();
  // TODO: Support updating the origin and normal yet.
}

bool pqSMTKSliceItemWidget::fetchOriginNormalAndInputsItems(
  smtk::attribute::DoubleItemPtr& originItem,
  smtk::attribute::DoubleItemPtr& normalItem,
  smtk::attribute::ReferenceItemPtr& inputsItem)
{
  auto groupItem = m_itemInfo.itemAs<smtk::attribute::GroupItem>();
  if (!groupItem || groupItem->numberOfGroups() < 1 || groupItem->numberOfItemsPerGroup() < 2)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Expected a group item with 1 group of 2 items.");
    return false;
  }
  std::string originItemName;
  std::string normalItemName;
  std::string inputsItemName;
  if (!m_itemInfo.component().attribute("Origin", originItemName))
  {
    originItemName = "Origin";
  }
  if (!m_itemInfo.component().attribute("Normal", normalItemName))
  {
    normalItemName = "Normal";
  }
  if (!m_itemInfo.component().attribute("Inputs", inputsItemName))
  {
    inputsItemName = "Inputs";
  }
  originItem = groupItem->findAs<smtk::attribute::DoubleItem>(originItemName);
  normalItem = groupItem->findAs<smtk::attribute::DoubleItem>(normalItemName);
  inputsItem = groupItem->findAs<smtk::attribute::ReferenceItem>(inputsItemName);
  if (!inputsItem)
  {
    // If a reference item is not specified, use the attribute's associations
    // (if associations are allowed).
    inputsItem = groupItem->attribute()->associations();
  }
  if (!originItem || !normalItem || !inputsItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find"
        << " a DoubleItem for the origin named \"" << originItemName << "\","
        << " a DoubleItem for the normal named \"" << normalItemName << "\","
        << " a ReferenceItem for the inputs named \"" << inputsItemName << "\","
        << " or some combination.");
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

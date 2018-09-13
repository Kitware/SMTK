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
#include "vtkPVXMLElement.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

using qtItem = smtk::extension::qtItem;
using AttributeItemInfo = smtk::extension::AttributeItemInfo;

pqSMTKBoxItemWidget::pqSMTKBoxItemWidget(
  const smtk::extension::AttributeItemInfo& info, Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
{
  this->createWidget();
}

pqSMTKBoxItemWidget::~pqSMTKBoxItemWidget()
{
}

qtItem* pqSMTKBoxItemWidget::createBoxItemWidget(const AttributeItemInfo& info)
{
  return new pqSMTKBoxItemWidget(info);
}

bool pqSMTKBoxItemWidget::createProxyAndWidget(
  vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget)
{
  // I. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "Box", server, "");
  if (!proxy)
  {
    return false;
  }
  widget = new pqBoxPropertyWidget(proxy, proxy->GetPropertyGroup(0));

  // II. Initialize the properties.
  // For now, since we want to map this to a vector of 6 doubles,
  // we do not allow rotation:
  auto widgetProxy = widget->widgetProxy();
  auto valueItem = this->itemAs<smtk::attribute::DoubleItem>();
  // FIXME! Determine bounds properly from scene if requested by m_itemInfo.
  // For now, just initialize the box using the item's values if they are
  // non-default (or the item has no default).
  if (valueItem && valueItem->numberOfValues() == 6)
  {
    if (!valueItem->hasDefault() || !valueItem->isUsingDefault())
    {
      double lo[3];
      double hi[3];
      auto valIt = valueItem->begin();
      for (int ii = 0; ii < 3; ++ii)
      {
        lo[ii] = *valIt;
        ++valIt;
        hi[ii] = *valIt;
        ++valIt;
      }
      vtkSMPropertyHelper(widgetProxy, "Position").Set(lo, 3);
      vtkSMPropertyHelper(widgetProxy, "Scale").Set(hi, 3);
    }
  }
  vtkSMPropertyHelper(widgetProxy, "RotationEnabled").Set(false);
  widgetProxy->UpdateVTKObjects();

  // III. Connect to signals so we know when the properties are modified.
  //      When they change, update the values in the SMTK item we represent.
  m_p->m_connector->Connect(
    proxy->GetProperty("Position"), vtkCommand::ModifiedEvent, this, SLOT(updateItemFromWidget()));
  m_p->m_connector->Connect(
    proxy->GetProperty("Scale"), vtkCommand::ModifiedEvent, this, SLOT(updateItemFromWidget()));
  // Not really needed yet:
  m_p->m_connector->Connect(
    proxy->GetProperty("Rotation"), vtkCommand::ModifiedEvent, this, SLOT(updateItemFromWidget()));

  return widget != nullptr;
}

/// Retrieve property values from ParaView proxy and store them in the attribute's Item.
void pqSMTKBoxItemWidget::updateItemFromWidget()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  auto valueItem = this->itemAs<smtk::attribute::DoubleItem>();
  if (!valueItem || valueItem->numberOfValues() != 6)
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Item widget has an update but the item does not exist or is not sized properly.");
    return;
  }

  vtkSMPropertyHelper originHelper(widget, "Position");
  vtkSMPropertyHelper lengthHelper(widget, "Scale");
  for (int i = 0; i < 3; ++i)
  {
    double lo = originHelper.GetAsDouble(i);
    double hi = lengthHelper.GetAsDouble(i);
    valueItem->setValue(2 * i, lo);
    valueItem->setValue(2 * i + 1, hi);
  }
}

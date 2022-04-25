//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKTransformWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/attribute/operators/Signal.h"

#include "smtk/extension/qt/qtOperationView.h"

#include "smtk/geometry/queries/BoundingBox.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Manager.h"

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

namespace
{
// A modification to pqBoxPropertyWidget has been developed concurrently with
// this class that adds a constructor argument to hide reference bounds
// information. This logic will use the new constructor argument if it is
// available.
template<typename MyClass>
class CanHideReferenceBounds
{
  template<typename X>
  static std::true_type testConstructorWithArgs(decltype(X{ std::declval<vtkSMProxy*>(),
                                                            std::declval<vtkSMPropertyGroup*>(),
                                                            std::declval<QWidget*>(),
                                                            std::declval<bool>() })*);
  template<typename X>
  static std::false_type testConstructorWithArgs(...);

public:
  using type = decltype(testConstructorWithArgs<MyClass>(nullptr));
  static constexpr bool value = type::value;
};

struct CreateAndHideReferenceBounds
{
  template<typename BoxPropertyWidget>
  typename std::enable_if<CanHideReferenceBounds<BoxPropertyWidget>::value, BoxPropertyWidget*>::
    type
    operator()(vtkSMProxy* smproxy, vtkSMPropertyGroup* smgroup)
  {
    return new BoxPropertyWidget(smproxy, smgroup, nullptr, true);
  }

  template<typename BoxPropertyWidget>
  typename std::enable_if<!CanHideReferenceBounds<BoxPropertyWidget>::value, BoxPropertyWidget*>::
    type
    operator()(vtkSMProxy* smproxy, vtkSMPropertyGroup* smgroup)
  {
    return new BoxPropertyWidget(smproxy, smgroup);
  }
};
} // namespace

using qtItem = smtk::extension::qtItem;
using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

struct pqSMTKTransformWidget::Internal
{
  smtk::operation::Observers::Key m_opObserver;
};

pqSMTKTransformWidget::pqSMTKTransformWidget(
  const smtk::extension::qtAttributeItemInfo& info,
  Qt::Orientation orient)
  : pqSMTKAttributeItemWidget(info, orient)
  , m_internal(new pqSMTKTransformWidget::Internal)
{

  QPointer<pqSMTKTransformWidget> guardedObject(this);
  m_internal->m_opObserver = info.baseView()->uiManager()->operationManager()->observers().insert(
    [guardedObject](
      const smtk::operation::Operation& op,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result res) {
      if (
        !guardedObject || !guardedObject->item() ||
        event != smtk::operation::EventType::DID_OPERATE ||
        op.index() == std::type_index(typeid(smtk::attribute::Signal)).hash_code())
      {
        return 0;
      }

      auto attribute = guardedObject->item()->attribute();
      if (!attribute)
      {
        return 0;
      }

      smtk::resource::PersistentObjectPtr object = attribute->associations()->value();

      if (res->findReference("modified")->contains(attribute->associations()->value()))
      {
        guardedObject->resetWidget();
      }
      return 0;
    },
    "pqSMTKTransformWidget: Reset widget if the item being transformed is modified.");

  this->createWidget();
}

pqSMTKTransformWidget::~pqSMTKTransformWidget() = default;

qtItem* pqSMTKTransformWidget::createTransformWidget(const qtAttributeItemInfo& info)
{
  return new pqSMTKTransformWidget(info);
}

bool pqSMTKTransformWidget::createProxyAndWidget(
  vtkSMProxy*& proxy,
  pqInteractivePropertyWidget*& widget)
{
  std::vector<smtk::attribute::DoubleItemPtr> items;
  smtk::attribute::StringItemPtr control;
  if (!this->fetchTransformItems(items, control))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not find items for widget.");
    return false;
  }

  // I. Create the ParaView widget and a proxy for its representation.
  pqApplicationCore* paraViewApp = pqApplicationCore::instance();
  pqServer* server = paraViewApp->getActiveServer();
  pqObjectBuilder* builder = paraViewApp->getObjectBuilder();

  proxy = builder->createProxy("implicit_functions", "Box", server, "");
  if (!proxy)
  {
    return false;
  }
  vtkSMPropertyHelper(proxy, "UseReferenceBounds").Set(true);

  // Construct a pqBoxPropertyWidget and hide its reference bounds if the version
  // of ParaView we are using supports it.
  widget = CreateAndHideReferenceBounds().operator()<pqBoxPropertyWidget>(
    proxy, proxy->GetPropertyGroup(0));

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
  this->resetWidget();
  auto* widgetProxy = widget->widgetProxy();
  widgetProxy->UpdateVTKObjects();

  return widget != nullptr;
}

void pqSMTKTransformWidget::resetWidget()
{
  if (!m_p->m_pvwidget)
  {
    return;
  }

  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();

  smtk::attribute::AttributePtr attribute = this->item()->attribute();

  if (attribute)
  {
    smtk::resource::PersistentObjectPtr object = attribute->associations()->value();
    try
    {
      auto& boundingBoxQuery = smtk::resource::queryForObject<smtk::geometry::BoundingBox>(*object);
      std::array<double, 6> boundingBox = boundingBoxQuery(object);
      vtkSMPropertyHelper(m_p->m_pvwidget->proxy(), "Bounds").Set(&boundingBox[0], 6);
    }
    catch (smtk::resource::query::BadTypeError&)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Associated object does not have a geometric bounding box query.");
    }
  }

  std::vector<smtk::attribute::DoubleItemPtr> items;
  smtk::attribute::StringItemPtr control;
  if (!this->fetchTransformItems(items, control))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Item widget has an update but the item(s) do not exist or are not sized properly.");
    return;
  }

  std::array<std::string, 3> names = { "Position", "Scale", "Rotation" };
  std::array<double, 3> defaults = { 0., 1., 0. };
  for (std::size_t i = 0; i < 3; ++i)
  {
    vtkVector3d vector;
    for (std::size_t j = 0; j < 3; ++j)
    {
      items[i]->setValue(j, defaults[i]);
      vector[static_cast<int>(j)] = defaults[i];
    }
    vtkSMPropertyHelper(widget, names[i].c_str()).Set(vector.GetData(), 3);
  }
}

void pqSMTKTransformWidget::updateItemFromWidgetInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  smtk::attribute::StringItemPtr control;
  if (!this->fetchTransformItems(items, control))
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

  std::array<std::string, 3> names = { "Position", "Scale", "Rotation" };
  for (std::size_t i = 0; i < 3; ++i)
  {
    vtkVector3d widgetParameter;
    vtkSMPropertyHelper(widget, names[i].c_str()).Get(widgetParameter.GetData(), 3);
    for (std::size_t j = 0; j < 3; ++j)
    {
      if (items[i]->value(j) != widgetParameter[static_cast<int>(j)])
      {
        didChange = true;
        items[i]->setValue(j, widgetParameter[static_cast<int>(j)]);
      }
    }
  }

  if (didChange)
  {
    Q_EMIT modified();
  }
}

void pqSMTKTransformWidget::updateWidgetFromItemInternal()
{
  vtkSMNewWidgetRepresentationProxy* widget = m_p->m_pvwidget->widgetProxy();
  std::vector<smtk::attribute::DoubleItemPtr> items;
  smtk::attribute::StringItemPtr control;
  if (!this->fetchTransformItems(items, control))
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
  std::array<std::string, 3> names = { "Position", "Scale", "Rotation" };
  for (std::size_t i = 0; i < 3; ++i)
  {
    vtkSMPropertyHelper(widget, names[i].c_str()).Set(&(*items[i]->begin()), 3);
  }
}

bool pqSMTKTransformWidget::fetchTransformItems(
  std::vector<smtk::attribute::DoubleItemPtr>& items,
  smtk::attribute::StringItemPtr& control)
{
  items.clear();
  control = nullptr;

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

  // Find items in the group based on names in the configuration info
  std::string positionName;
  std::string scaleName;
  std::string rotationName;
  std::array<std::string, 3> names = { "Position", "Scale", "Rotation" };
  for (std::size_t i = 0; i < 3; ++i)
  {
    m_itemInfo.component().attribute(names[i], names[i]);
    items.push_back(groupItem->findAs<smtk::attribute::DoubleItem>(names[i]));
  }

  return true;
}

void pqSMTKTransformWidget::setControlState(
  const std::string& controlState,
  QCheckBox* controlWidget)
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

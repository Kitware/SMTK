//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidgetP.h"

#include "smtk/extension/qt/qtBaseAttributeView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/model/EntityRef.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"

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

#include <QEvent>

using namespace smtk::attribute;
using qtItem = smtk::extension::qtItem;

pqSMTKAttributeItemWidget::OverrideWhen pqSMTKAttributeItemWidget::OverrideWhenConvert(
  const std::string& str)
{
  if (str == "Never" || str == "never")
  {
    return OverrideWhen::Never;
  }
  return OverrideWhen::Unset;
}

std::string pqSMTKAttributeItemWidget::OverrideWhenConvert(OverrideWhen val)
{
  switch (val)
  {
    case OverrideWhen::Unset:
      return "Unset";
      break;
    case OverrideWhen::Never:
      return "Never";
      break;
    default:
      break;
  }
  return "Invalid";
}

pqSMTKAttributeItemWidget::FallbackStrategy pqSMTKAttributeItemWidget::FallbackStrategyConvert(
  const std::string& str)
{
  if (str == "Force" || str == "force")
  {
    return FallbackStrategy::Force;
  }
  return FallbackStrategy::Hide;
}

std::string pqSMTKAttributeItemWidget::FallbackStrategyConvert(FallbackStrategy val)
{
  switch (val)
  {
    case FallbackStrategy::Hide:
      return "Hide";
      break;
    case FallbackStrategy::Force:
      return "Force";
      break;
    default:
      break;
  }
  return "Invalid";
}

pqSMTKAttributeItemWidget::GeometrySource pqSMTKAttributeItemWidget::GeometrySourceConvert(
  const std::string& str)
{
  if (str == "Item" || str == "item")
  {
    return GeometrySource::Item;
  }
  else if (str == "Associations" || str == "associations")
  {
    return GeometrySource::Associations;
  }
  else if (str == "Links" || str == "links")
  {
    return GeometrySource::Links;
  }
  else if (str == "Scene" || str == "scene")
  {
    return GeometrySource::Scene;
  }
  else if (str == "None" || str == "none")
  {
    return GeometrySource::None;
  }
  return GeometrySource::BestGuess;
}

std::string pqSMTKAttributeItemWidget::GeometrySourceConvert(GeometrySource val)
{
  switch (val)
  {
    case GeometrySource::Item:
      return "Item";
      break;
    case GeometrySource::Associations:
      return "Associations";
      break;
    case GeometrySource::Links:
      return "Links";
      break;
    case GeometrySource::Scene:
      return "Scene";
      break;
    case GeometrySource::BestGuess:
      return "BestGuess";
      break;
    case GeometrySource::None:
      return "None";
      break;
    default:
      break;
  }
  return "Invalid";
}

pqSMTKAttributeItemWidget::pqSMTKAttributeItemWidget(
  const smtk::extension::qtAttributeItemInfo& info, Qt::Orientation orient)
  : qtItem(info)
{
  // Subclass constructors must call:
  // this->createWidget();

  m_p = new Internal(info.item(), this->widget(), info.baseView(), orient);
  m_isLeafItem = true;
  std::string ow;
  std::string fs;
  std::string gs;
  if (m_itemInfo.component().attribute("OverrideWhen", ow))
  {
    ow = "Unset";
  }
  if (m_itemInfo.component().attribute("FallbackStrategy", fs))
  {
    fs = "Hide";
  }
  if (m_itemInfo.component().attribute("GeometrySource", gs))
  {
    gs = "BestGuess";
  }
  m_p->m_overrideWhen = pqSMTKAttributeItemWidget::OverrideWhenConvert(ow);
  m_p->m_fallbackStrategy = pqSMTKAttributeItemWidget::FallbackStrategyConvert(fs);
  m_p->m_geometrySource = pqSMTKAttributeItemWidget::GeometrySourceConvert(gs);
  m_p->m_opObserver = info.baseView()->uiManager()->operationManager()->observers().insert(
    [&](const smtk::operation::Operation& op, smtk::operation::EventType event,
      smtk::operation::Operation::Result res) {
      if (event == smtk::operation::EventType::DID_OPERATE &&
        dynamic_cast<const smtk::attribute::Signal*>(&op) &&
        res->findReference("modified")->contains(this->item()->attribute()))
      {
        this->updateWidgetFromItem();
      }
      return 0;
    },
    "pqSMTKAttributeItemWidget: Update widget from item when parent attribute is modified.");
}

pqSMTKAttributeItemWidget::pqSMTKAttributeItemWidget(smtk::attribute::ItemPtr itm, QWidget* p,
  smtk::extension::qtBaseView* bview, Qt::Orientation orient)
  : qtItem(smtk::extension::qtAttributeItemInfo(itm, smtk::view::View::Component(), p, bview))
{
  m_p = new Internal(itm, this->widget(), bview, orient);
  m_p->m_opObserver = bview->uiManager()->operationManager()->observers().insert(
    [&](const smtk::operation::Operation& op, smtk::operation::EventType event,
      smtk::operation::Operation::Result res) {
      if (event == smtk::operation::EventType::DID_OPERATE &&
        dynamic_cast<const smtk::attribute::Signal*>(&op) &&
        res->findReference("modified")->contains(this->item()->attribute()))
      {
        this->updateWidgetFromItem();
      }
      return 0;
    },
    "pqSMTKAttributeItemWidget: Update widget from item when parent attribute is modified.");
  m_isLeafItem = true;
  this->createWidget();
}

pqSMTKAttributeItemWidget::~pqSMTKAttributeItemWidget()
{
  auto ui = this->uiManager();
  auto operationManager = ui ? ui->operationManager() : nullptr;
  if (operationManager && m_p->m_opObserver.assigned())
  {
    operationManager->observers().erase(m_p->m_opObserver);
  }
  delete this->m_p;
  this->m_p = NULL;
}

pqInteractivePropertyWidget* pqSMTKAttributeItemWidget::propertyWidget()
{
  return m_p->m_pvwidget;
}

void pqSMTKAttributeItemWidget::updateItemFromWidget()
{
  if (m_p->m_state == Internal::State::Idle)
  {
    m_p->m_state = Internal::State::UpdatingFromAttribute;
    this->updateItemFromWidgetInternal();

    // Only return to idle after event queue is processed
    // since the "modified()" signal's connected slots must
    // be allowed to run before changing state.
    QTimer::singleShot(1, [this]() { m_p->m_state = Internal::State::Idle; });
  }
}

void pqSMTKAttributeItemWidget::updateWidgetFromItem()
{
  if (m_p->m_state == Internal::State::Idle)
  {
    m_p->m_state = Internal::State::UpdatingFromUI;
    this->updateWidgetFromItemInternal();

    // Only return to idle after event queue is processed
    // since the "modified()" signal's connected slots must
    // be allowed to run before changing state.
    QTimer::singleShot(1, [this]() { m_p->m_state = Internal::State::Idle; });
  }
}

void pqSMTKAttributeItemWidget::setOutputOptional(int optionEnabled)
{
  bool enabled = !!optionEnabled;
  // Update the item.
  auto item = m_itemInfo.item();
  item->setIsEnabled(enabled);
  // Now update the UI by hiding the widget representation.
  if (enabled)
  {
    m_p->m_pvwidget->select();
    m_p->m_pvwidget->show();
  }
  else
  {
    m_p->m_pvwidget->deselect();
    m_p->m_pvwidget->hide();
  }
  emit modified();
}

/// Create Qt widgets as required (may be called multiple times if Item is reconfigured).
void pqSMTKAttributeItemWidget::updateItemData()
{
  this->updateUI();
  this->qtItem::updateItemData();
}

void pqSMTKAttributeItemWidget::acceptWidgetValues()
{
  this->m_p->m_pvwidget->apply();
}

bool pqSMTKAttributeItemWidget::eventFilter(QObject* obj, QEvent* event)
{
  if (obj != this)
  {
    bool isHide = event && event->type() == QEvent::Hide;
    bool isShow = event && event->type() == QEvent::Show;
    pqInteractivePropertyWidget* ww = this->propertyWidget();
    if (obj == ww && (isHide || isShow))
    {
      this->update3DWidgetVisibility(isShow);
    }
  }
  return this->qtItem::eventFilter(obj, event);
}

void pqSMTKAttributeItemWidget::update3DWidgetVisibility(bool visible)
{
  pqInteractivePropertyWidget* ww = this->propertyWidget();
  if (ww)
  {
    ww->setWidgetVisible(visible);
  }
}

/// Initialize Qt widgets used to represent our smtk::attribute::Item.
void pqSMTKAttributeItemWidget::createWidget()
{
  smtk::attribute::ItemPtr dataObj = this->item();
  auto iview = dynamic_cast<smtk::extension::qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }

  this->clearChildWidgets();
  this->updateItemData();
}

/**\brief Remove existing widgets in order to prepare for reconfiguration.
  *
  * If conditional children exist, this may get called after createWidget().
  */
void pqSMTKAttributeItemWidget::clearChildWidgets()
{
}

/// Actually create widgets for whole of Item (label, editor, and conditional children).
void pqSMTKAttributeItemWidget::updateUI()
{
  auto dataObj = this->item();
  auto iview = dynamic_cast<smtk::extension::qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }

  m_widget = new QFrame(this->parentWidget());
  this->m_p->m_layout = new QGridLayout(m_widget);
  this->m_p->m_layout->setMargin(0);
  this->m_p->m_layout->setSpacing(0);
  this->m_p->m_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if (dataObj->isOptional())
  {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(optionalCheck);
  }

  auto dataObjDef = dataObj->definition();
  QString labelText;
  if (!dataObj->label().empty())
  {
    labelText = dataObj->label().c_str();
  }
  else
  {
    labelText = dataObj->name().c_str();
  }
  QLabel* label = new QLabel(labelText, m_widget);
  label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  const std::string strBriefDescription = dataObjDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    label->setToolTip(strBriefDescription.c_str());
  }

  // auto valueItem = smtk::dynamic_pointer_cast<ValueItem>(dataObj);
  auto valueItemDef = dynamic_cast<const ValueItemDefinition*>(dataObj->definition().get());
  if (valueItemDef && !valueItemDef->units().empty())
  {
    QString unitText = label->text();
    unitText.append(" (").append(valueItemDef->units().c_str()).append(")");
    label->setText(unitText);
  }
  if (dataObjDef->advanceLevel() && m_itemInfo.baseView())
  {
    label->setFont(m_itemInfo.baseView()->uiManager()->advancedFont());
  }
  labelLayout->addWidget(label);
  this->m_p->m_label = label;

  this->createEditor();
  this->m_p->m_layout->addLayout(labelLayout, 0, 0);

  if (this->parentWidget() && this->parentWidget()->layout())
  {
    this->parentWidget()->layout()->addWidget(m_widget);
  }
  if (dataObj->isOptional())
  {
    this->setOutputOptional(dataObj->isEnabled() ? 1 : 0);
  }
}

/// Create the widget(s) that allow editing of the Item (as opposed to labels and conditional child widgets).
void pqSMTKAttributeItemWidget::createEditor()
{
  auto item = this->item();
  if (!item)
  {
    return;
  }

  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);

  // remove previous signal-slot links
  m_p->m_connector->Disconnect();

  vtkSMProxy* source;
  pqInteractivePropertyWidget* pvwidget;
  if (!this->createProxyAndWidget(source, pvwidget))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not create ParaView widget.");
    return;
  }

  pqActiveObjects& actives(pqActiveObjects::instance());
  pvwidget->setView(actives.activeView());
  m_p->m_pvwidget = pvwidget;
  // changeFinished is emitted when users modify properties using Qt widgets:
  QObject::connect(pvwidget, SIGNAL(changeFinished()), this, SLOT(updateItemFromWidget()));
  // interaction/endInteraction are emitted when users modify properties using ParaView widgets:
  QObject::connect(pvwidget, SIGNAL(interaction()), this, SLOT(updateItemFromWidget()));
  QObject::connect(pvwidget, SIGNAL(endInteraction()), this, SLOT(acceptWidgetValues()));
  // When the active view changes, move the widget to that view.
  QObject::connect(&actives, SIGNAL(viewChanged(pqView*)), pvwidget, SLOT(setView(pqView*)));

  // Install an event filter on the widget to detect visibility changes so
  // we can give 3-d widgets the opportunity to hide themselves when the Qt
  // widget is hidden.
  pvwidget->installEventFilter(this);

  editorLayout->addWidget(pvwidget);
  this->m_p->m_layout->addLayout(editorLayout, 0, 1);
  pvwidget->show();
  pvwidget->select();

  this->updateWidgetFromItem();
  // Finally, hide widgets not in currently-displayed tabs
  this->update3DWidgetVisibility(pvwidget->isVisible());
}

bool pqSMTKAttributeItemWidget::validateControlItem(const smtk::attribute::StringItemPtr& item)
{
  if (!item)
  {
    return false;
  }
  auto def =
    smtk::dynamic_pointer_cast<const smtk::attribute::StringItemDefinition>(item->definition());
  if (!def || !def->isDiscrete())
  {
    return false;
  }
  std::size_t index;
  if ((!def->getEnumIndex("active", index) && !def->getEnumIndex("Active", index)) ||
    // NB: Allow but do not require "visible" enum
    (!def->getEnumIndex("inactive", index) && !def->getEnumIndex("Inactive", index)))
  {
    return false;
  }
  return true;
}

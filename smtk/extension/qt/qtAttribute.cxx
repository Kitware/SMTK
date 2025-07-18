//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtAttribute.h"

#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtUnitsLineEdit.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/PathGrammar.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/StringUtil.h"

#include "smtk/io/Logger.h"

#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>

#include <cstdlib> // for atexit()

#define DEBUG_ATTRIBUTE 0

using namespace smtk::attribute;
using namespace smtk::extension;

class qtAttributeInternals
{
public:
  enum UnitsMode
  {
    Editable,
    ViewOnly,
    None
  };
  qtAttributeInternals(
    smtk::attribute::AttributePtr myAttribute,
    const smtk::view::Configuration::Component& comp,
    QWidget* p,
    qtBaseAttributeView* myView)
  {
    m_parentWidget = p;
    m_attribute = myAttribute;
    m_view = myView;
    m_attComp = comp;

    // Does the component representing the attribute contain a Style block?
    int sindex = m_attComp.findChild("ItemViews");
    if (sindex == -1)
    {
      return;
    }
    auto iviews = m_attComp.child(sindex);
    qtAttributeItemInfo::buildFromComponent(iviews, m_view, m_itemViewMap);
    std::string unitsStr;
    if (m_attComp.attribute("UnitsMode", unitsStr))
    {
      smtk::common::StringUtil::lower(unitsStr);
      if (unitsStr == "none")
      {
        m_unitsMode = UnitsMode::None;
      }
      else if (unitsStr == "viewonly")
      {
        m_unitsMode = UnitsMode::ViewOnly;
      }
      else if (unitsStr == "editable")
      {
        m_unitsMode = UnitsMode::Editable;
      }
    }
  }

  ~qtAttributeInternals() = default;
  smtk::attribute::WeakAttributePtr m_attribute;
  QPointer<QWidget> m_parentWidget;
  QList<smtk::extension::qtItem*> m_items;
  QPointer<qtBaseAttributeView> m_view;
  smtk::view::Configuration::Component m_attComp;
  std::map<std::string, qtAttributeItemInfo> m_itemViewMap;
  UnitsMode m_unitsMode = UnitsMode::Editable;
};

qtAttribute::qtAttribute(
  smtk::attribute::AttributePtr myAttribute,
  const smtk::view::Configuration::Component& comp,
  QWidget* p,
  qtBaseView* myView,
  bool createWidgetWhenEmpty)
{
  auto* attView = dynamic_cast<qtBaseAttributeView*>(myView);
  m_internals = new qtAttributeInternals(myAttribute, comp, p, attView);
  m_widget = nullptr;
  m_useSelectionManager = false;
  this->createWidget(createWidgetWhenEmpty);
}

qtAttribute::~qtAttribute()
{
  // First Clear all the items
  this->removeItems();
  delete m_widget;

  delete m_internals;
}

void qtAttribute::removeItems()
{
  for (int i = 0; i < m_internals->m_items.count(); i++)
  {
    m_internals->m_items.value(i)->markForDeletion();
  }

  m_internals->m_items.clear();
}

void qtAttribute::createWidget(bool createWidgetWhenEmpty)
{
  // A qtAttribute is empty if it has no items to display and if it
  // does not need to display its units.  Assume that it is empty and then
  // updated it if that is not the case.
  m_isEmpty = true;

  auto att = this->attribute();
  if (!createWidgetWhenEmpty)
  {
    if (
      (att == nullptr) ||
      ((att->numberOfItems() == 0) && (att->associations() == nullptr) &&
       ((!att->supportsUnits()) ||
        (m_internals->m_unitsMode == qtAttributeInternals::UnitsMode::None))))
    {
      return;
    }

    // Based on the View, see if the attribute should be displayed
    if (!m_internals->m_view->displayAttribute(att))
    {
      return;
    }

    // We will always display units if there are any associated with the attribute and we were told to display them else
    // we need to see if any items would be displayed
    if (
      (!att->supportsUnits()) ||
      (m_internals->m_unitsMode == qtAttributeInternals::UnitsMode::None))
    {
      int numShowItems = 0;
      std::size_t i, n = att->numberOfItems();
      if (m_internals->m_view)
      {
        for (i = 0; i < n; i++)
        {
          if (m_internals->m_view->displayItem(att->item(static_cast<int>(i))))
          {
            numShowItems++;
          }
        }
        // also check associations
        if (m_internals->m_view->displayItem(att->associations()))
        {
          numShowItems++;
        }
      }
      else // show everything
      {
        numShowItems = static_cast<int>(att->associations() ? n + 1 : n);
      }
      if (numShowItems == 0)
      {
        return;
      }
    }
  }

  m_isEmpty = false;
  QFrame* attFrame = new QFrame(this->parentWidget());
  attFrame->setObjectName(att->name().c_str());
  attFrame->setFrameShape(QFrame::Box);
  m_widget = attFrame;

  QVBoxLayout* layout = new QVBoxLayout(m_widget);
  layout->setMargin(3);
  m_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
}

void qtAttribute::addItem(qtItem* child)
{
  if (!m_internals->m_items.contains(child))
  {
    m_internals->m_items.append(child);
    // When the item is modified so is the attribute that uses it
    connect(child, &qtItem::modified, [this](qtItem* item) {
      if (item != nullptr)
      {
        Q_EMIT this->itemModified(item);
        // make sure this object is still valid after the above signal is processed
        if (m_internals != nullptr)
        {
          Q_EMIT this->modified();
        }
        return;
      }
      // See if the sender itself is from a qtItem and if so return that item
      auto* iobject = qobject_cast<smtk::extension::qtItem*>(this->sender());
      if (iobject)
      {
        Q_EMIT this->itemModified(iobject);
        // make sure this object is still valid after the above signal is processed
        if (m_internals != nullptr)
        {
          Q_EMIT this->modified();
        }
      }
    });
  }
}

QList<qtItem*>& qtAttribute::items() const
{
  return m_internals->m_items;
}

void qtAttribute::showAdvanceLevelOverlay(bool show)
{
  for (int i = 0; i < m_internals->m_items.count(); i++)
  {
    m_internals->m_items.value(i)->showAdvanceLevelOverlay(show);
  }
}

void qtAttribute::createBasicLayout(bool includeAssociations)
{
  // Initially we have not displayed any items
  m_isEmpty = true;
  //If there is no main widget there is nothing to show
  if (!m_widget)
  {
    return;
  }
  QLayout* layout = m_widget->layout();
  qtItem* qItem = nullptr;
  smtk::attribute::AttributePtr att = this->attribute();
  auto* uiManager = m_internals->m_view->uiManager();

  // If the attribute has units and we were told not to ignore them then display them
  if (att->supportsUnits() && (m_internals->m_unitsMode != qtAttributeInternals::UnitsMode::None))
  {
    QFrame* unitFrame = new QFrame(this->m_widget);
    std::string unitFrameName = att->name() + "UnitsFrame";
    unitFrame->setObjectName(unitFrameName.c_str());
    unitFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout* unitsLayout = new QHBoxLayout(unitFrame);
    std::string unitsLayoutName = att->name() + "UnitsLayout";
    unitsLayout->setObjectName(unitsLayoutName.c_str());
    unitsLayout->setMargin(0);
    unitFrame->setLayout(unitsLayout);
    std::string unitsLabel = att->name() + "'s units:";

    // Are we suppose to allow editing?
    if (m_internals->m_unitsMode == qtAttributeInternals::UnitsMode::Editable)
    {
      QLabel* uLabel = new QLabel(unitsLabel.c_str(), unitFrame);
      unitsLayout->addWidget(uLabel);

      // If the attribute's definition has explicit units use them else use those set explicitly on the attribute
      QString baseUnits = (att->definition()->units() == "*") ? att->units().c_str()
                                                              : att->definition()->units().c_str();
      auto* unitsWidget =
        new qtUnitsLineEdit(baseUnits, att->definition()->unitSystem(), uiManager, unitFrame);
      std::string unitsWidgetName = att->name() + "UnitsLineWidget";
      unitsWidget->setObjectName(unitsWidgetName.c_str());
      unitsLayout->addWidget(unitsWidget);
      unitsWidget->setAndClassifyText(att->units().c_str());
      // Create a regular expression validator to prevent leading spaces
      QObject::connect(
        unitsWidget, &qtUnitsLineEdit::editingCompleted, this, &qtAttribute::onUnitsChanged);
    }
    else // Units are to be displayed read only
    {
      unitsLabel.append(att->units());
      QLabel* uLabel = new QLabel(unitsLabel.c_str(), unitFrame);
      unitsLayout->addWidget(uLabel);
    }
    layout->addWidget(unitFrame);
    m_isEmpty = false;
  }
  // If there are model associations for the attribute, create UI for them if requested.
  // This will be the same widget used for ModelEntityItem.
  if (includeAssociations && att->associations())
  {
    // Allow the association widget to be overridden the same as other items.
    auto assoc = att->associatedObjects();
    auto it = m_internals->m_itemViewMap.find(assoc->name());
    if (it != m_internals->m_itemViewMap.end())
    {
      auto info = it->second;
      info.setParentWidget(m_widget);
      info.setItem(assoc);
      qItem = uiManager->createItem(info);
    }
    else
    {
      smtk::view::Configuration::Component comp; // not currently used but will be
      qtAttributeItemInfo info(att->associations(), comp, m_widget, m_internals->m_view);
      qItem = uiManager->createItem(info);
    }
    if (qItem && qItem->widget())
    {
      m_isEmpty = false;
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
    }
  }
  // Now go through all child items and create ui components.
  std::size_t i, n = att->numberOfItems();
  for (i = 0; i < n; i++)
  {
    // Does this item have a style associated with it?
    auto item = att->item(static_cast<int>(i));
    auto it = m_internals->m_itemViewMap.find(item->name());
    if (it != m_internals->m_itemViewMap.end())
    {
      auto info = it->second;
      info.setParentWidget(m_widget);
      info.setItem(item);
      qItem = uiManager->createItem(info);
    }
    else
    {
      smtk::view::Configuration::Component comp; // not current used but will be
      qtAttributeItemInfo info(item, comp, m_widget, m_internals->m_view);
      qItem = uiManager->createItem(info);
    }
    if (qItem && qItem->widget())
    {
      m_isEmpty = false;
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
    }
  }
}

smtk::attribute::AttributePtr qtAttribute::attribute() const
{
  return m_internals->m_attribute.lock();
}

QWidget* qtAttribute::parentWidget()
{
  return m_internals->m_parentWidget;
}

bool qtAttribute::isEmpty() const
{
  return m_isEmpty;
}

bool qtAttribute::isValid() const
{
  auto att = this->attribute();
  if (att)
  {
    return att->isValid();
  }
  return true;
}

void qtAttribute::onUnitsChanged(QString newUnits)
{
  auto att = this->attribute();
  if (att && att->setLocalUnits(newUnits.toStdString()))
  {
    Q_EMIT this->modified();
  }
}

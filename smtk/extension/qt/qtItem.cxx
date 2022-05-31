//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtItem.h"

#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include <QComboBox>
#include <QLayout>

using namespace smtk::attribute;
using namespace smtk::extension;

class qtItemInternals
{
public:
  qtItemInternals() = default;
  ~qtItemInternals() = default;
  QPointer<qtOverlayFilter> advOverlay;
  QPointer<QComboBox> AdvLevelCombo;
};

qtItem::qtItem(const qtAttributeItemInfo& info)
  : m_itemInfo(info)
{
  this->Internals = new qtItemInternals();
  this->m_widget = nullptr;
  this->m_isLeafItem = false;
  m_useSelectionManager = false;
  m_readOnly = m_itemInfo.component().attributeAsBool("ReadOnly");
  m_markedForDeletion = false;
}

qtItem::~qtItem()
{
  this->clearChildItems();
  delete m_widget;
  delete this->Internals;
}

smtk::attribute::ResourcePtr qtItem::attributeResource() const
{
  return m_itemInfo.baseView()->attributeResource();
}

void qtItem::markForDeletion()
{
  // Disconnect this object's signals
  disconnect(this, nullptr, nullptr, nullptr);
  // Indicate that this object will be deleted
  m_markedForDeletion = true;
  this->deleteLater();
}

void qtItem::updateItemData() {}

void qtItem::addChildItem(qtItem* child)
{
  if (!m_childItems.contains(child))
  {
    m_childItems.append(child);
  }
}

void qtItem::removeChildItem(qtItem* child)
{
  // There should only be one instance of the child
  if (m_childItems.removeOne(child))
  {
    child->markForDeletion();
  }
}

QList<qtItem*>& qtItem::childItems()
{
  return m_childItems;
}

void qtItem::clearChildItems()
{
  for (int i = 0; i < m_childItems.count(); i++)
  {
    m_childItems.value(i)->markForDeletion();
  }
  m_childItems.clear();
}

void qtItem::showAdvanceLevelOverlay(bool show)
{
  if (!m_widget)
  {
    return;
  }
  if (show && !this->Internals->advOverlay)
  {
    this->Internals->advOverlay = new qtOverlayFilter(m_widget, this);
    this->Internals->AdvLevelCombo = new QComboBox(this->Internals->advOverlay->overlay());
    this->Internals->advOverlay->overlay()->addOverlayWidget(this->Internals->AdvLevelCombo);
    m_itemInfo.uiManager()->initAdvanceLevels(this->Internals->AdvLevelCombo);
    int mylevel = this->item()->localAdvanceLevel(0);
    bool foundLevel = false;
    std::set<int> levels;
    for (int i = 0; i < this->Internals->AdvLevelCombo->count(); i++)
    {
      int level = this->Internals->AdvLevelCombo->itemData(i).toInt();
      if (level == mylevel)
      {
        this->Internals->AdvLevelCombo->setCurrentIndex(i);
        foundLevel = true;
        break;
      }
      levels.insert(level);
    }
    // in case the advanceLevel is not one of the "AdvanceLevels" specified in xml,
    // we use the lowest level that is more than mylevel.
    // for example, levels 100, 200, 300.
    // mylevel 50 will use level 100
    // mylevel 150 will use level 200
    // my level 250 will use level 300
    // my level 350 will not be visible at all
    if (!foundLevel && !levels.empty())
    {
      levels.insert(mylevel);
      std::set<int>::iterator it = levels.upper_bound(mylevel);
      mylevel = it == levels.end() ? *(--it) : *it;
      int idx = std::distance(levels.begin(), it) - 1;
      this->Internals->AdvLevelCombo->setCurrentIndex(idx);
    }
    const double* rgba = this->attributeResource()->advanceLevelColor(mylevel);
    if (rgba)
    {
      this->Internals->advOverlay->overlay()->setColor(
        QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]));
    }

    QObject::connect(
      this->Internals->AdvLevelCombo,
      SIGNAL(currentIndexChanged(int)),
      this,
      SLOT(onAdvanceLevelChanged(int)));
  }

  if (this->Internals->advOverlay)
  {
    this->Internals->advOverlay->setActive(show);
  }
  if (show)
  {
    m_widget->repaint();
  }
}

void qtItem::onAdvanceLevelChanged(int levelIdx)
{
  unsigned int level = this->Internals->AdvLevelCombo->itemData(levelIdx).toUInt();
  this->setLocalAdvanceLevel(level);
}

void qtItem::setLocalAdvanceLevel(unsigned int l)
{
  auto item = m_itemInfo.item();
  item->setLocalAdvanceLevel(0, l);
  item->setLocalAdvanceLevel(1, l);
  if (this->Internals->advOverlay)
  {
    const double* rgba = this->attributeResource()->advanceLevelColor(l);
    if (rgba)
    {
      this->Internals->advOverlay->overlay()->setColor(
        QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]));
      this->Internals->advOverlay->overlay()->repaint();
    }
  }
}

bool qtItem::isReadOnly() const
{
  auto item = m_itemInfo.item();
  auto* view = m_itemInfo.baseView();
  return (
    (m_readOnly || (item == nullptr) || (view == nullptr)) ? true : !view->isItemWriteable(item));
}

bool qtItem::isFixedWidth() const
{
  return false;
}

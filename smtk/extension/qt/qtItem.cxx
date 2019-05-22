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

#include "smtk/extension/qt/qtBaseView.h"
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
  qtItemInternals() {}
  ~qtItemInternals() {}
  QPointer<qtOverlayFilter> advOverlay;
  QPointer<QComboBox> AdvLevelCombo;
};

qtItem::qtItem(const qtAttributeItemInfo& info)
  : m_itemInfo(info)
{
  this->Internals = new qtItemInternals();
  this->m_widget = NULL;
  this->m_isLeafItem = false;
  m_useSelectionManager = false;
  m_readOnly = m_itemInfo.component().attributeAsBool("ReadOnly");
}

qtItem::~qtItem()
{
  this->clearChildItems();
  if (m_widget)
  {
    delete m_widget;
  }
  if (this->Internals)
  {
    delete this->Internals;
  }
}

void qtItem::updateItemData()
{
}

void qtItem::addChildItem(qtItem* child)
{
  if (!m_childItems.contains(child))
  {
    m_childItems.append(child);
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
    delete m_childItems.value(i);
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
    int mylevel = this->item()->advanceLevel(0);
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
    if (!foundLevel && levels.size() > 0)
    {
      levels.insert(mylevel);
      std::set<int>::iterator it = levels.upper_bound(mylevel);
      mylevel = it == levels.end() ? *(--it) : *it;
      int idx = std::distance(levels.begin(), it) - 1;
      this->Internals->AdvLevelCombo->setCurrentIndex(idx);
    }
    const double* rgba = m_itemInfo.uiManager()->attResource()->advanceLevelColor(mylevel);
    if (rgba)
    {
      this->Internals->advOverlay->overlay()->setColor(
        QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]));
    }

    QObject::connect(this->Internals->AdvLevelCombo, SIGNAL(currentIndexChanged(int)), this,
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
  int level = this->Internals->AdvLevelCombo->itemData(levelIdx).toInt();
  this->setAdvanceLevel(level);
}

void qtItem::setAdvanceLevel(int l)
{
  auto item = m_itemInfo.item();
  item->setAdvanceLevel(0, l);
  item->setAdvanceLevel(1, l);
  if (this->Internals->advOverlay)
  {
    const double* rgba = m_itemInfo.uiManager()->attResource()->advanceLevelColor(l);
    if (rgba)
    {
      this->Internals->advOverlay->overlay()->setColor(
        QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]));
      this->Internals->advOverlay->overlay()->repaint();
    }
  }
}

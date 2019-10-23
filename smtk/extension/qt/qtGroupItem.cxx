//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtGroupItem.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include <QCoreApplication>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QPointer>
#include <QScrollBar>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

using namespace smtk::extension;

class qtGroupItemInternals
{
public:
  QPointer<QFrame> ChildrensFrame;
  QMap<QToolButton*, QList<qtItem*> > ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
  QPointer<QTableWidget> ItemsTable;
  std::map<std::string, qtAttributeItemInfo> m_itemViewMap;
};

qtItem* qtGroupItem::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::GroupItem>() == nullptr)
  {
    return nullptr;
  }
  return new qtGroupItem(info);
}

qtGroupItem::qtGroupItem(const qtAttributeItemInfo& info)
  : qtItem(info)
{
  this->Internals = new qtGroupItemInternals;
  m_itemInfo.createNewDictionary(this->Internals->m_itemViewMap);
  m_isLeafItem = true;
  std::string insertMode;
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();

  // We support prepending subgroups iff the group is extensible and
  // the insertion mode is set to prepend
  m_prependMode =
    (item->isExtensible() && m_itemInfo.component().attribute("InsertMode", insertMode) &&
      ((insertMode == "prepend") || (insertMode == "Prepend")));

  this->createWidget();
}

qtGroupItem::~qtGroupItem()
{
  this->clearChildItems();
  delete this->Internals;
}

void qtGroupItem::setLabelVisible(bool visible)
{
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (item == nullptr)
  {
    return;
  }
  if (!item || !item->numberOfGroups())
  {
    return;
  }

  QGroupBox* groupBox = qobject_cast<QGroupBox*>(m_widget);
  groupBox->setTitle(visible ? item->label().c_str() : "");
}

void qtGroupItem::createWidget()
{
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (item == nullptr)
  {
    return;
  }
  this->clearChildItems();
  if ((!item->numberOfGroups() && !item->isExtensible()))
  {
    return;
  }

  QString title = item->label().c_str();
  QGroupBox* groupBox = new QGroupBox(title, m_itemInfo.parentWidget());
  m_widget = groupBox;

  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  // Instantiate a layout for the widget, but do *not* assign it to a variable.
  // because that would cause a compiler warning, since the layout is not
  // explicitly referenced anywhere in this scope. (There is no memory
  // leak because the layout instance is parented by the widget.)
  new QVBoxLayout(m_widget);
  m_widget->layout()->setMargin(0);
  this->Internals->ChildrensFrame = new QFrame(groupBox);
  new QVBoxLayout(this->Internals->ChildrensFrame);

  m_widget->layout()->addWidget(this->Internals->ChildrensFrame);

  if (m_itemInfo.parentWidget())
  {
    m_itemInfo.parentWidget()->layout()->setAlignment(Qt::AlignTop);
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  this->updateItemData();

  // If the group is optional, we need a checkbox
  if (item->isOptional())
  {
    groupBox->setCheckable(true);
    groupBox->setChecked(item->isEnabled());
    this->Internals->ChildrensFrame->setVisible(item->isEnabled());
    connect(groupBox, SIGNAL(toggled(bool)), this, SLOT(setEnabledState(bool)));
  }
}

void qtGroupItem::setEnabledState(bool checked)
{
  this->Internals->ChildrensFrame->setVisible(checked);
  auto item = m_itemInfo.item();
  if (item == nullptr)
  {
    return;
  }

  if (checked != item->isEnabled())
  {
    item->setIsEnabled(checked);
    emit this->modified();
    auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
    if (iview)
    {
      iview->valueChanged(item);
    }
  }
}

void qtGroupItem::updateItemData()
{
  this->clearChildItems();
  qDeleteAll(this->Internals->ChildrensFrame->findChildren<QWidget*>("groupitem_frame"));
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (!item || (!item->numberOfGroups() && !item->isExtensible()))
  {
    return;
  }

  std::size_t i, n = item->numberOfGroups();
  if (item->isExtensible())
  {
    //clear mapping
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
    if (this->Internals->ItemsTable)
    {
      this->Internals->ItemsTable->blockSignals(true);
      this->Internals->ItemsTable->clear();
      this->Internals->ItemsTable->setRowCount(0);
      this->Internals->ItemsTable->setColumnCount(0);
      this->Internals->ItemsTable->blockSignals(false);
    }

    // The new item button
    if (!this->Internals->AddItemButton)
    {
      this->Internals->AddItemButton = new QToolButton(this->Internals->ChildrensFrame);
      this->Internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      QString iconName(":/icons/attribute/plus.png");
      std::string extensibleLabel = "Add Row";
      m_itemInfo.component().attribute("ExtensibleLabel", extensibleLabel);
      this->Internals->AddItemButton->setText(extensibleLabel.c_str());
      this->Internals->AddItemButton->setIcon(QIcon(iconName));
      this->Internals->AddItemButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      connect(this->Internals->AddItemButton, SIGNAL(clicked()), this, SLOT(onAddSubGroup()));
      this->Internals->ChildrensFrame->layout()->addWidget(this->Internals->AddItemButton);
    }
    m_widget->layout()->setSpacing(3);
  }

  for (i = 0; i < n; i++)
  {
    int subIdx = static_cast<int>(i);
    if (item->isExtensible())
    {
      this->addItemsToTable(subIdx);
    }
    else
    {
      this->addSubGroup(subIdx);
    }
  }
  this->qtItem::updateItemData();
}

void qtGroupItem::onAddSubGroup()
{
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (!item || (!item->numberOfGroups() && !item->isExtensible()))
  {
    return;
  }
  if (m_prependMode)
  {
    if (item->prependGroup())
    {
      this->addItemsToTable(0);
      emit this->widgetSizeChanged();
      emit this->modified();
    }
  }
  else
  {
    if (item->appendGroup())
    {
      int subIdx = static_cast<int>(item->numberOfGroups()) - 1;
      if (item->isExtensible())
      {
        this->addItemsToTable(subIdx);
      }
      else
      {
        this->addSubGroup(subIdx);
      }
      emit this->widgetSizeChanged();
      emit this->modified();
    }
  }
}

void qtGroupItem::addSubGroup(int i)
{
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (!item || (!item->numberOfGroups() && !item->isExtensible()))
  {
    return;
  }
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (!iview)
  {
    return;
  }

  const std::size_t numItems = item->numberOfItemsPerGroup();
  QBoxLayout* frameLayout = qobject_cast<QBoxLayout*>(this->Internals->ChildrensFrame->layout());
  QFrame* subGroupFrame = new QFrame(this->Internals->ChildrensFrame);
  subGroupFrame->setObjectName("groupitem_frame");
  QBoxLayout* subGroupLayout = new QVBoxLayout(subGroupFrame);
  if (item->numberOfGroups() == 1)
  {
    subGroupLayout->setMargin(0);
    subGroupFrame->setFrameStyle(QFrame::NoFrame);
  }
  else
  {
    frameLayout->setMargin(0);
    subGroupFrame->setFrameStyle(QFrame::Panel);
  }
  subGroupLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  QList<qtItem*> itemList;

  auto groupDef = item->definitionAs<attribute::GroupItemDefinition>();
  QString subGroupString;
  if (groupDef->hasSubGroupLabels())
  {
    subGroupString = QString::fromStdString(groupDef->subGroupLabel(i));
    QLabel* subGroupLabel = new QLabel(subGroupString, subGroupFrame);
    subGroupLayout->addWidget(subGroupLabel);
  }

  QList<smtk::attribute::ItemDefinitionPtr> childDefs;
  for (std::size_t j = 0; j < numItems; j++)
  {
    smtk::attribute::ConstItemDefinitionPtr itDef =
      item->item(i, static_cast<int>(j))->definition();
    childDefs.push_back(smtk::const_pointer_cast<attribute::ItemDefinition>(itDef));
  }
  const int tmpLen = m_itemInfo.uiManager()->getWidthOfItemsMaxLabel(
    childDefs, m_itemInfo.uiManager()->advancedFont());
  const int currentLen = iview->fixedLabelWidth();
  iview->setFixedLabelWidth(tmpLen);

  for (std::size_t j = 0; j < numItems; j++)
  {
    auto citem = item->item(i, static_cast<int>(j));
    auto it = Internals->m_itemViewMap.find(citem->name());
    qtItem* childItem;
    if (it != Internals->m_itemViewMap.end())
    {
      auto info = it->second;
      info.setParentWidget(m_widget);
      info.setItem(citem);
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    else
    {
      smtk::view::View::Component comp; // lets create a default style (an empty component)
      qtAttributeItemInfo info(citem, comp, m_widget, m_itemInfo.baseView());
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    if (childItem)
    {
      this->addChildItem(childItem);
      subGroupLayout->addWidget(childItem->widget());
      itemList.push_back(childItem);
      connect(childItem, SIGNAL(modified()), this, SLOT(onChildItemModified()));
    }
  }
  this->calculateTableHeight();
  iview->setFixedLabelWidth(currentLen);
  frameLayout->addWidget(subGroupFrame);
  this->onChildWidgetSizeChanged();
}

void qtGroupItem::onRemoveSubGroup()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(QObject::sender());
  if (!minusButton || !this->Internals->ExtensibleMap.contains(minusButton))
  {
    return;
  }

  int gIdx = this->Internals->MinusButtonIndices.indexOf(
    minusButton); //minusButton->property("SubgroupIndex").toInt();
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfGroups()))
  {
    return;
  }

  foreach (qtItem* qi, this->Internals->ExtensibleMap.value(minusButton))
  {
    // We need to remove the child from our list
    this->removeChildItem(qi);
  }
  //  delete this->Internals->ExtensibleMap.value(minusButton).first;
  this->Internals->ExtensibleMap.remove(minusButton);

  item->removeGroup(gIdx);
  int rowIdx = -1, rmIdx = -1;
  // normally rowIdx is same as gIdx, but we need to find
  // explicitly since minusButton could be NULL in MinusButtonIndices
  foreach (QToolButton* tb, this->Internals->MinusButtonIndices)
  {
    rowIdx = tb != NULL ? rowIdx + 1 : rowIdx;
    if (tb == minusButton)
    {
      rmIdx = rowIdx;
      break;
    }
  }
  if (rmIdx >= 0 && rmIdx < this->Internals->ItemsTable->rowCount())
  {
    this->Internals->ItemsTable->removeRow(rmIdx);
  }
  this->Internals->MinusButtonIndices.removeOne(minusButton);
  delete minusButton;
  this->calculateTableHeight();
  this->updateExtensibleState();
  emit this->modified();
}

void qtGroupItem::updateExtensibleState()
{
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (!item || !item->isExtensible())
  {
    return;
  }
  bool maxReached =
    (item->maxNumberOfGroups() > 0) && (item->maxNumberOfGroups() == item->numberOfGroups());
  this->Internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredGroups() > 0) &&
    (item->numberOfRequiredGroups() == item->numberOfGroups());
  foreach (QToolButton* tButton, this->Internals->ExtensibleMap.keys())
  {
    tButton->setEnabled(!minReached);
  }
}

void qtGroupItem::addItemsToTable(int index)
{
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (!item || !item->isExtensible())
  {
    return;
  }

  std::size_t j, m = item->numberOfItemsPerGroup();
  QBoxLayout* frameLayout = qobject_cast<QBoxLayout*>(this->Internals->ChildrensFrame->layout());
  if (!this->Internals->ItemsTable)
  {
    this->Internals->ItemsTable = new qtTableWidget(this->Internals->ChildrensFrame);
    this->Internals->ItemsTable->horizontalHeader()->setStretchLastSection(true);
    this->Internals->ItemsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->Internals->ItemsTable->setColumnCount(1); // for minus button
    frameLayout->addWidget(this->Internals->ItemsTable);
  }

  this->Internals->ItemsTable->blockSignals(true);
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QList<qtItem*> itemList;
  int added = 0;
  for (j = 0; j < m; j++)
  {
    auto citem = item->item(index, static_cast<int>(j));
    auto it = Internals->m_itemViewMap.find(citem->name());
    qtItem* childItem;
    if (it != Internals->m_itemViewMap.end())
    {
      auto info = it->second;
      info.setParentWidget(m_widget);
      info.setItem(citem);
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    else
    {
      smtk::view::View::Component comp; // lets create a default style (an empty component)
      qtAttributeItemInfo info(citem, comp, m_widget, m_itemInfo.baseView());
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    if (childItem)
    {
      this->addChildItem(childItem);
      if (added == 0)
      {
        this->Internals->ItemsTable->insertRow(index);
      }
      int numCols = this->Internals->ItemsTable->columnCount() - 1;
      if (added >= numCols)
      {
        this->Internals->ItemsTable->insertColumn(numCols + 1);
        std::string strItemLabel = citem->label().empty() ? citem->name() : citem->label();
        this->Internals->ItemsTable->setHorizontalHeaderItem(
          numCols + 1, new QTableWidgetItem(strItemLabel.c_str()));
      }
      childItem->setLabelVisible(false);
      this->Internals->ItemsTable->setCellWidget(index, added + 1, childItem->widget());
      itemList.push_back(childItem);
      connect(childItem, SIGNAL(widgetSizeChanged()), this, SLOT(onChildWidgetSizeChanged()),
        Qt::QueuedConnection);
      added++;
      connect(childItem, SIGNAL(modified()), this, SLOT(onChildItemModified()));
    }
  }
  QToolButton* minusButton = NULL;
  // if there are items
  if (added > 0)
  {
    minusButton = new QToolButton(this->Internals->ChildrensFrame);
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(16, 16));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove Row");
    //QVariant vdata(static_cast<int>(i));
    //minusButton->setProperty("SubgroupIndex", vdata);
    connect(minusButton, SIGNAL(clicked()), this, SLOT(onRemoveSubGroup()));
    this->Internals->ItemsTable->setCellWidget(index, 0, minusButton);

    this->Internals->ExtensibleMap[minusButton] = itemList;
  }
  this->Internals->MinusButtonIndices.insert(index, minusButton);
  this->updateExtensibleState();

  this->calculateTableHeight();
  this->Internals->ItemsTable->blockSignals(false);
  this->onChildWidgetSizeChanged();
}

void qtGroupItem::onChildWidgetSizeChanged()
{
  if (this->Internals->ItemsTable)
  {
    this->Internals->ItemsTable->resizeColumnsToContents();
    this->Internals->ItemsTable->resizeRowsToContents();
    emit this->widgetSizeChanged();
  }
}

/* Slot for properly emitting signals when an attribute's item is modified */
void qtGroupItem::onChildItemModified()
{
  emit this->modified();
}

void qtGroupItem::calculateTableHeight()
{
  if (this->Internals->ItemsTable == nullptr)
  {
    return;
  }

  int n = this->Internals->ItemsTable->verticalHeader()->count();
  int totalHeight = this->Internals->ItemsTable->horizontalScrollBar()->height() +
    this->Internals->ItemsTable->horizontalHeader()->height();
  for (int i = 0; i < n; i++)
  {
    totalHeight += this->Internals->ItemsTable->verticalHeader()->sectionSize(i);
  }
  this->Internals->ItemsTable->setMinimumHeight(totalHeight);
}

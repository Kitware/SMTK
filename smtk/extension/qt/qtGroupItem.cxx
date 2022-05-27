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

#include "smtk/io/Logger.h"
#include "smtk/io/attributeUtils.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QMessageBox>
#include <QPointer>
#include <QScrollBar>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

using namespace smtk::extension;

class qtGroupItemInternals
{
public:
  QPointer<QFrame> ButtonsFrame;
  QPointer<QFrame> ChildrensFrame;
  QMap<QToolButton*, QList<qtItem*>> ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
  QPointer<QTableWidget> ItemsTable;
  QPointer<QFrame> m_mainFrame;
  QPointer<QFrame> m_contentsFrame;
  QPointer<QFrame> m_titleFrame;
  QPointer<QCheckBox> m_titleCheckbox;
  QPointer<QLabel> m_titleLabel;
  QPointer<QLabel> m_alertLabel;
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
  m_internals = new qtGroupItemInternals;
  m_itemInfo.createNewDictionary(m_internals->m_itemViewMap);
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
  delete m_internals;
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

  m_internals->m_titleFrame->setVisible(visible);
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
  // The structure of a qtGroupItem
  // - Main widget is m_mainFrame using a VBox Layout
  //   which contains two sub-frames
  //   - m_titleFrame: for optional check-box, group label, invalidity icon
  //   - m_contentsFrame: contains the following
  //     - m_buttonsFrame: for extensible group items
  //     - m_childrensFrame: for group's children
  m_internals->m_mainFrame = new QFrame(m_itemInfo.parentWidget());
  m_internals->m_mainFrame->setObjectName("qtGroupItem");
  m_widget = m_internals->m_mainFrame;
  m_widget->setObjectName(item->name().c_str());
  auto* mainLayout = new QVBoxLayout(m_widget);
  mainLayout->setObjectName("mainLayout");
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);
  mainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_internals->m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  m_internals->m_titleFrame = new QFrame(m_internals->m_mainFrame);
  m_internals->m_mainFrame->setObjectName("TitleFrame");
  mainLayout->addWidget(m_internals->m_titleFrame);
  auto* titleLayout = new QHBoxLayout(m_internals->m_titleFrame);
  titleLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  titleLayout->setMargin(0);

  // We create both a checkbox and plain label for the group item since
  // its optional property could change while it is being displayed via
  // forceRequired - we let the groupItem's current optional state determine
  // when the checkbox is displayed
  QString title = item->label().c_str();

  m_internals->m_titleCheckbox = new QCheckBox(m_internals->m_titleFrame);
  m_internals->m_titleCheckbox->setObjectName("TitleCB");
  titleLayout->addWidget(m_internals->m_titleCheckbox);
  connect(
    m_internals->m_titleCheckbox, &QCheckBox::stateChanged, this, &qtGroupItem::setEnabledState);

  m_internals->m_titleLabel = new QLabel(title, m_internals->m_titleFrame);
  m_internals->m_titleLabel->setObjectName("Title");
  titleLayout->addWidget(m_internals->m_titleLabel);

  m_internals->m_alertLabel = new QLabel(m_internals->m_titleFrame);
  m_internals->m_alertLabel->setObjectName("Alert");
  titleLayout->addWidget(m_internals->m_alertLabel);
  int height = m_itemInfo.uiManager()->alertPixmap().height();
  QPixmap alert = m_itemInfo.uiManager()->alertPixmap().scaledToHeight(height * 0.5);
  m_internals->m_alertLabel->setPixmap(alert);

  m_internals->m_contentsFrame = new QFrame(m_internals->m_mainFrame);
  m_internals->m_contentsFrame->setObjectName("Contents");
  mainLayout->addWidget(m_internals->m_contentsFrame);
  auto* contentsLayout = new QVBoxLayout(m_internals->m_contentsFrame);
  contentsLayout->setObjectName("contentsLayout");
  contentsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  // Lets indent the contents a bit to the right.
  contentsLayout->setContentsMargins(10, 0, 0, 0);
  m_internals->m_contentsFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
  m_internals->m_contentsFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  m_internals->ButtonsFrame = new QFrame(m_internals->m_contentsFrame);
  m_internals->ButtonsFrame->setObjectName("ButtonFrame");
  auto* hLayout = new QHBoxLayout(m_internals->ButtonsFrame);
  hLayout->setObjectName("hLayout");
  contentsLayout->addWidget(m_internals->ButtonsFrame);
  hLayout->setMargin(0);

  m_internals->ChildrensFrame = new QFrame(m_internals->m_contentsFrame);
  m_internals->ChildrensFrame->setObjectName("ChildrensFrame");
  auto* vLayout = new QVBoxLayout(m_internals->ChildrensFrame);
  vLayout->setObjectName("vLayout");
  contentsLayout->addWidget(m_internals->ChildrensFrame);
  vLayout->setMargin(0);

  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }

  if (m_itemInfo.parentWidget())
  {
    m_itemInfo.parentWidget()->layout()->setAlignment(Qt::AlignTop);
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  this->updateItemData();

  // If the group is optional, we need a check box
  if (item->isOptional())
  {
    m_internals->m_titleCheckbox->setVisible(true);
    m_internals->m_titleCheckbox->setChecked(item->localEnabledState());
    auto* iview = m_itemInfo.baseView();
    bool enforceCategories = (iview) ? (!iview->ignoreCategories()) : true;
    m_internals->m_contentsFrame->setVisible(
      item->localEnabledState() &&
      item->hasRelevantChildren(enforceCategories, true, this->uiManager()->advanceLevel()));
  }
  else
  {
    m_internals->m_titleCheckbox->setVisible(false);
  }
}

void qtGroupItem::setEnabledState(int state)
{
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (item == nullptr)
  {
    return;
  }

  auto* iview = m_itemInfo.baseView();
  bool enabled = (state == Qt::Checked);
  bool enforceCategories = (iview) ? (!iview->ignoreCategories()) : true;
  m_internals->m_contentsFrame->setVisible(
    enabled &&
    item->hasRelevantChildren(enforceCategories, true, this->uiManager()->advanceLevel()));
  if (enabled != item->localEnabledState())
  {
    item->setIsEnabled(enabled);
    Q_EMIT this->modified();
    if (iview)
    {
      iview->valueChanged(item);
    }
  }
}

void qtGroupItem::updateItemData()
{
  // Since an item's optional status can change (using
  // forceRequired) we need to reevaluate the optional status
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (item->isOptional())
  {
    m_internals->m_titleCheckbox->blockSignals(true);
    m_internals->m_titleCheckbox->setVisible(true);
    m_internals->m_titleCheckbox->setChecked(item->localEnabledState());
    m_internals->m_titleCheckbox->blockSignals(false);
  }
  else
  {
    m_internals->m_titleCheckbox->setVisible(false);
  }
  this->clearChildItems();
  auto myChildren = m_internals->ChildrensFrame->findChildren<QWidget*>("groupitemFrame");
  for (auto* myChild : myChildren)
  {
    myChild->deleteLater();
  }

  if (!item || (!item->numberOfGroups() && !item->isExtensible()))
  {
    return;
  }

  std::size_t i, n = item->numberOfGroups();
  if (item->isExtensible())
  {
    //clear mapping
    m_internals->ExtensibleMap.clear();
    m_internals->MinusButtonIndices.clear();
    if (m_internals->ItemsTable)
    {
      m_internals->ItemsTable->blockSignals(true);
      m_internals->ItemsTable->clear();
      m_internals->ItemsTable->setRowCount(0);
      m_internals->ItemsTable->setColumnCount(1);
      m_internals->ItemsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(" "));
      m_internals->ItemsTable->blockSignals(false);
    }

    // The new item button
    if (!m_internals->AddItemButton)
    {
      m_internals->AddItemButton = new QToolButton(m_internals->ButtonsFrame);
      m_internals->AddItemButton->setObjectName("AddItemButton");
      m_internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      QString iconName(":/icons/attribute/plus.png");
      std::string extensibleLabel = "Add Row";
      m_itemInfo.component().attribute("ExtensibleLabel", extensibleLabel);
      m_internals->AddItemButton->setText(extensibleLabel.c_str());
      m_internals->AddItemButton->setIcon(QIcon(iconName));
      m_internals->AddItemButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      connect(m_internals->AddItemButton, SIGNAL(clicked()), this, SLOT(onAddSubGroup()));
      m_internals->ButtonsFrame->layout()->addWidget(m_internals->AddItemButton);
      qobject_cast<QHBoxLayout*>(m_internals->ButtonsFrame->layout())->addStretch();

      // Do we show a load File option?
      if (item->isExtensible() && m_itemInfo.component().attributeAsBool("ImportFromFile"))
      {
        std::string buttonText = "Load from File";
        m_itemInfo.component().attribute("LoadButtonText", buttonText);
        auto* loadButton = new QToolButton(m_internals->ButtonsFrame);
        loadButton->setObjectName("loadFileButton");
        loadButton->setText(buttonText.c_str());
        connect(loadButton, SIGNAL(clicked(bool)), this, SLOT(onImportFromFile()));
        m_internals->ButtonsFrame->layout()->addWidget(loadButton);
      }
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
  this->updateValidityStatus();
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
      Q_EMIT this->widgetSizeChanged();
      Q_EMIT this->modified();
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
      Q_EMIT this->widgetSizeChanged();
      Q_EMIT this->modified();
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
  auto* iview = m_itemInfo.baseView();
  if (!iview)
  {
    return;
  }

  const std::size_t numItems = item->numberOfItemsPerGroup();
  QBoxLayout* frameLayout = qobject_cast<QBoxLayout*>(m_internals->ChildrensFrame->layout());
  QFrame* subGroupFrame = new QFrame(m_internals->ChildrensFrame);
  subGroupFrame->setObjectName(QString("groupitemFrame%1").arg(i));
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
    subGroupLabel->setObjectName(QString("subGroupLabel%1").arg(i));
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
    auto it = m_internals->m_itemViewMap.find(citem->name());
    qtItem* childItem;
    if (it != m_internals->m_itemViewMap.end())
    {
      auto info = it->second;
      info.setParentWidget(m_widget);
      info.setItem(citem);
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    else
    {
      smtk::view::Configuration::Component comp; // lets create a default style (an empty component)
      qtAttributeItemInfo info(citem, comp, m_widget, m_itemInfo.baseView());
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    if (childItem)
    {
      this->addChildItem(childItem);
      subGroupLayout->addWidget(childItem->widget());
      itemList.push_back(childItem);

      // Make unique to prevent this connection from being made more than once upon updating the gui.
      connect(
        childItem, SIGNAL(modified()), this, SLOT(onChildItemModified()), Qt::UniqueConnection);
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
  if (!minusButton || !m_internals->ExtensibleMap.contains(minusButton))
  {
    return;
  }

  int gIdx = m_internals->MinusButtonIndices.indexOf(
    minusButton); //minusButton->property("SubgroupIndex").toInt();
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if (!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfGroups()))
  {
    return;
  }

  Q_FOREACH (qtItem* qi, m_internals->ExtensibleMap.value(minusButton))
  {
    // We need to remove the child from our list
    this->removeChildItem(qi);
  }
  //  delete m_internals->ExtensibleMap.value(minusButton).first;
  m_internals->ExtensibleMap.remove(minusButton);

  item->removeGroup(gIdx);
  int rowIdx = -1, rmIdx = -1;
  // normally rowIdx is same as gIdx, but we need to find
  // explicitly since minusButton could be nullptr in MinusButtonIndices
  Q_FOREACH (QToolButton* tb, m_internals->MinusButtonIndices)
  {
    rowIdx = tb != nullptr ? rowIdx + 1 : rowIdx;
    if (tb == minusButton)
    {
      rmIdx = rowIdx;
      break;
    }
  }
  if (rmIdx >= 0 && rmIdx < m_internals->ItemsTable->rowCount())
  {
    m_internals->ItemsTable->removeRow(rmIdx);
  }
  m_internals->MinusButtonIndices.removeOne(minusButton);
  delete minusButton;
  this->calculateTableHeight();
  this->updateExtensibleState();
  Q_EMIT this->modified();
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
  m_internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredGroups() > 0) &&
    (item->numberOfRequiredGroups() == item->numberOfGroups());
  Q_FOREACH (QToolButton* tButton, m_internals->ExtensibleMap.keys())
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

  QBoxLayout* frameLayout = qobject_cast<QBoxLayout*>(m_internals->ChildrensFrame->layout());
  if (!m_internals->ItemsTable)
  {
    m_internals->ItemsTable = new qtTableWidget(m_internals->ChildrensFrame);
    m_internals->ItemsTable->setObjectName(QString("ItemsTable%1").arg(index));
    m_internals->ItemsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_internals->ItemsTable->setColumnCount(1); // for minus button
    m_internals->ItemsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(" "));
    m_internals->ItemsTable->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::ResizeToContents);
    m_internals->ItemsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    frameLayout->addWidget(m_internals->ItemsTable);
  }

  m_internals->ItemsTable->blockSignals(true);
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QList<qtItem*> itemList;
  int added = 0;
  // We need to determine if the last column needs to stretch to fill extra space.
  // The conditions are:
  // 1. All of the qtItems are of fixedWidth or number of qtItems are 0
  // 2. The last column is not of fixed width - there seems to be a bug in
  // QT when setting the last column to be Interactive - The table does not
  // properly expand to fill the area provided.  Setting the last column to
  // stretch seems to fix this.
  bool stretchLastColumn = true;
  std::size_t j, m = item->numberOfItemsPerGroup();
  for (j = 0; j < m; j++)
  {
    auto citem = item->item(index, static_cast<int>(j));
    auto it = m_internals->m_itemViewMap.find(citem->name());
    qtItem* childItem;
    if (it != m_internals->m_itemViewMap.end())
    {
      auto info = it->second;
      info.setParentWidget(m_widget);
      info.setItem(citem);
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    else
    {
      smtk::view::Configuration::Component comp; // lets create a default style (an empty component)
      qtAttributeItemInfo info(citem, comp, m_widget, m_itemInfo.baseView());
      childItem = m_itemInfo.uiManager()->createItem(info);
    }
    if (childItem)
    {
      this->addChildItem(childItem);
      if (added == 0)
      {
        m_internals->ItemsTable->insertRow(index);
      }
      int numCols = m_internals->ItemsTable->columnCount() - 1;
      if (added >= numCols)
      {
        m_internals->ItemsTable->insertColumn(numCols + 1);
        std::string strItemLabel = citem->label().empty() ? citem->name() : citem->label();
        m_internals->ItemsTable->setHorizontalHeaderItem(
          numCols + 1, new QTableWidgetItem(strItemLabel.c_str()));
        if (childItem->isFixedWidth())
        {
          m_internals->ItemsTable->horizontalHeader()->setSectionResizeMode(
            numCols + 1, QHeaderView::ResizeToContents);
        }
        else
        {
          m_internals->ItemsTable->horizontalHeader()->setSectionResizeMode(
            numCols + 1, QHeaderView::Interactive);
          stretchLastColumn = false;
        }
      }
      childItem->setLabelVisible(false);
      m_internals->ItemsTable->setCellWidget(index, added + 1, childItem->widget());
      itemList.push_back(childItem);
      connect(
        childItem,
        SIGNAL(widgetSizeChanged()),
        this,
        SLOT(onChildWidgetSizeChanged()),
        Qt::QueuedConnection);
      added++;
      connect(
        childItem,
        SIGNAL(modified()),
        this,
        SLOT(onChildItemModified()),
        static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    }
  }
  // Check to see if the last column is not fixed width and set the  table to stretch
  // the last column if that is the case
  if (!(itemList.isEmpty() || itemList.back()->isFixedWidth()))
  {
    stretchLastColumn = true;
  }
  m_internals->ItemsTable->horizontalHeader()->setStretchLastSection(stretchLastColumn);
  QToolButton* minusButton = nullptr;
  // if there are items
  if (added > 0)
  {
    minusButton = new QToolButton(m_internals->ChildrensFrame);
    minusButton->setObjectName(QString("minusButton%1").arg(index));
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(16, 16));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove Row");
    //QVariant vdata(static_cast<int>(i));
    //minusButton->setProperty("SubgroupIndex", vdata);
    connect(minusButton, SIGNAL(clicked()), this, SLOT(onRemoveSubGroup()));
    m_internals->ItemsTable->setCellWidget(index, 0, minusButton);

    m_internals->ExtensibleMap[minusButton] = itemList;
  }
  m_internals->MinusButtonIndices.insert(index, minusButton);
  this->updateExtensibleState();

  this->calculateTableHeight();
  m_internals->ItemsTable->blockSignals(false);
  this->onChildWidgetSizeChanged();
}

void qtGroupItem::onChildWidgetSizeChanged()
{
  if (m_internals->ItemsTable)
  {
    // There seems to be a bug in QT - if you ask the table to
    // resize all of its columns to fit their data, the table may
    // not fill up the space provided.  resizing each column seperately
    // does not seem to have this issue.
    int i, n = m_internals->ItemsTable->columnCount();
    for (i = 0; i < n; i++)
    {
      m_internals->ItemsTable->resizeColumnToContents(i);
    }
    // We need to resize the height of the cell incase the
    // child's height has changed
    m_internals->ItemsTable->resizeRowsToContents();
    Q_EMIT this->widgetSizeChanged();
  }
}

/* Slot for properly emitting signals when an attribute's item is modified */
void qtGroupItem::onChildItemModified()
{
  this->updateValidityStatus();
  //Get the qtItem that sent the signal
  qtItem* item = qobject_cast<qtItem*>(this->sender());

  if (item != nullptr)
  {
    Q_EMIT this->childModified(item);
    Q_EMIT this->modified();
  }
}

void qtGroupItem::updateValidityStatus()
{
  // If this item has been marked for deletion
  // we don't need to do anything
  if (m_markedForDeletion)
  {
    return;
  }

  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  if ((!item->isConditional()) || item->conditionalsSatisfied())
  {
    m_internals->m_alertLabel->setVisible(false);
    return;
  }
  m_internals->m_alertLabel->setVisible(true);
}

void qtGroupItem::calculateTableHeight()
{
  if (m_internals->ItemsTable == nullptr)
  {
    return;
  }
  int numRows = -1; // Set the height to be the entire table
  m_itemInfo.component().attributeAsInt("MinNumberOfRows", numRows);

  if (numRows == -1)
  {
    numRows = m_internals->ItemsTable->verticalHeader()->count();
  }

  int totalHeight = m_internals->ItemsTable->horizontalScrollBar()->height() +
    m_internals->ItemsTable->horizontalHeader()->height();
  for (int i = 0; i < numRows; i++)
  {
    totalHeight += m_internals->ItemsTable->verticalHeader()->sectionSize(i);
  }
  m_internals->ItemsTable->setMinimumHeight(totalHeight);
}

void qtGroupItem::onImportFromFile()
{
  smtk::io::Logger logger;
  std::string sep(","), comment("#"), format("csv"), broweserTitle("Load from File..."),
    fileExten("Data Files (*.csv *.dat *.txt);;All files (*.*)");
  m_itemInfo.component().attribute("ValueSeparator", sep);
  m_itemInfo.component().attribute("CommentChar", comment);
  m_itemInfo.component().attribute("FileFormat", format);
  m_itemInfo.component().attribute("BrowserTitle", broweserTitle);
  m_itemInfo.component().attribute("FileExtensions", fileExten);
  QString fname =
    QFileDialog::getOpenFileName(m_widget, tr(broweserTitle.c_str()), "", tr(fileExten.c_str()));
  if (fname.isEmpty())
  {
    return;
  }
  auto item = m_itemInfo.itemAs<attribute::GroupItem>();
  // Lets make the format be case insensitive
  std::transform(format.begin(), format.end(), format.begin(), ::tolower);
  if (format == "csv")
  {
    if (smtk::io::importFromCSV(*item, fname.toStdString(), logger, false, sep, comment))
    {
      this->updateItemData();
      Q_EMIT this->modified();
    }
  }
  else if (format == "double")
  {
    if (smtk::io::importFromDoubleFile(*item, fname.toStdString(), logger, false, sep, comment))
    {
      this->updateItemData();
      Q_EMIT this->modified();
    }
  }
  else
  {
    smtkErrorMacro(logger, "Unsupported File Format: " << format);
  }
  if (logger.numberOfRecords())
  {
    QMessageBox::warning(m_widget, tr("GroupItem Import Log)"), logger.convertToString().c_str());
  }
}

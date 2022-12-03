//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/markup/qtOntologyItem.h"

#include "smtk/extension/paraview/markup/qtOntologyModel.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Information.h"

#include "smtk/markup/Resource.h"
#include "smtk/markup/ontology/Source.h"
#include "smtk/markup/operators/TagIndividual.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/Regex.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QFont>
#include <QFontInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QSizePolicy>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

#include <cassert>

#include "ui_qtOntologyItem.h"

using namespace smtk::attribute;
using namespace smtk::extension;

class qtOntologyItem::Internal : public Ui_ontologyTagView
{
public:
  Internal(qtOntologyItem* self)
    : m_self(self)
  {
  }
  ~Internal() = default;

  bool addTermToTagList(const QModelIndex& index)
  {
    auto nameValue =
      index.sibling(index.row(), static_cast<int>(qtOntologyModel::Column::Name)).data().toString();
    auto preExisting = m_addedTagList->findItems(nameValue, Qt::MatchExactly);
    if (!preExisting.isEmpty())
    {
      return false;
    }

    int rowIndex = m_addedTagList->rowCount();
    m_addedTagList->insertRow(rowIndex);
    auto* tagNameItem = new QTableWidgetItem(nameValue);
    tagNameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    tagNameItem->setText(nameValue);
    m_addedTagList->setItem(rowIndex, 0, tagNameItem);
    return true;
  }

  smtk::markup::Resource* resourceFromAssociations(
    const smtk::attribute::ReferenceItem::Ptr& assocs)
  {
    smtk::markup::Resource* result = nullptr;
    for (const auto& assoc : *assocs)
    {
      if (auto* rsrc = dynamic_cast<smtk::markup::Resource*>(assoc.get()))
      {
        result = rsrc;
        break;
      }
      else if (auto* comp = dynamic_cast<smtk::markup::Component*>(assoc.get()))
      {
        if (auto* rsrc = dynamic_cast<smtk::markup::Resource*>(comp->parentResource()))
        {
          result = rsrc;
          break;
        }
      }
    }
    return result;
  }

  /// Copy the list of ontology classes to disconnect from the UI into the
  /// operation's parameters.
  void copyRemovableTagsToParameter(const ComponentItem::Ptr& tagsToRemove)
  {
    int rr = m_removableTagList->rowCount();
    // Resize tagsToRemove to be big enough to remove every tag (since we
    // don't know how many are checked yet).
    tagsToRemove->setNumberOfValues(rr);
    auto* resource = this->resourceFromAssociations(tagsToRemove->attribute()->associations());
    if (!resource)
    {
      return;
    }
    int tt = 0;
    for (int ii = 0; ii < rr; ++ii)
    {
      auto* item = m_removableTagList->item(ii, 0);
      if (item->checkState() == Qt::Checked)
      {
        auto tagsForName = resource->findByName<std::vector<smtk::markup::OntologyIdentifier*>>(
          item->text().toStdString());
        if (!tagsForName.empty())
        {
          if (tagsToRemove->setValue(tt, tagsForName.front()->shared_from_this()))
          {
            ++tt;
          }
        }
      }
    }
    // Now, prune the list to only the tags we set.
    tagsToRemove->setNumberOfValues(tt);
  }

  qtOntologyItem* m_self{ nullptr };
  QPointer<QCheckBox> m_enable;
  QPointer<QLabel> m_label;
  QPointer<QWidget> m_content;
  QPointer<QVBoxLayout> m_layout;
  QPointer<QAction> m_removeTagAction;
  QPointer<QAction> m_removeAllAction;
  QPointer<QCompleter> m_completer;
  QPointer<qtOntologyModel> m_model;
  QString m_lastTerm;
  std::string m_ontologyName;
  std::string m_ontologyURL;
};

qtItem* qtOntologyItem::createItemWidget(const qtAttributeItemInfo& info)
{
  // Check attribute::Item instances exist as specified in \a info.
  // 1. Our "primary" item is a string item holding the identifier's name.
  smtk::attribute::ItemPtr item = info.item();
  if (!item || !std::dynamic_pointer_cast<GroupItem>(item))
  {
    return nullptr;
  }
  // 2. TODO: Check that ontology URL and other configuration exist and are valid.
  return new qtOntologyItem(info);
}

qtOntologyItem::qtOntologyItem(const qtAttributeItemInfo& info)
  : qtItem(info)
{
  m_p = new Internal(this);
  this->createWidget();
}

qtOntologyItem::~qtOntologyItem()
{
  delete m_p;
}

bool qtOntologyItem::setOntologyModel(const std::string& modelName)
{
  const auto& source = smtk::markup::ontology::Source::findByName(modelName);
  if (source.url().empty() || source.name().empty())
  {
    return false;
  }
  qtOntologyModel* model = new qtOntologyModel(source, m_p->m_name);
  m_p->m_ontologyName = source.name();
  m_p->m_ontologyURL = source.url();
  return this->setOntologyModel(model);
}

bool qtOntologyItem::setOntologyModel(qtOntologyModel* model)
{
  if (!model || model == m_p->m_model)
  {
    return false;
  }
  m_p->m_model = model;
  m_p->m_completer->setModel(m_p->m_model);
  m_p->m_name->setCompleter(m_p->m_completer);
  return true;
}

void qtOntologyItem::modelEntryHighlighted(const QModelIndex& index)
{
  auto text = index.sibling(index.row(), static_cast<int>(qtOntologyModel::Column::Description))
                .data()
                .toString();
  auto url =
    index.sibling(index.row(), static_cast<int>(qtOntologyModel::Column::URL)).data().toString();
  QString description =
    QString(
      R"(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html>
<head>
  <meta name="qrichtext" content="1" />
  <style type="text/css">
    p, li { white-space: pre-wrap; }
    a { font-family: 'monospace'; font-size: 8pt; color: navy; text-decoration: none; }
  </style>
</head>
<body style="font-family:'Serif'; font-size:9pt; font-weight:400; font-style:normal;">
<p>%1</p>
<p style=" text-align: right; "><a href="%2">%2</a></p>
</body>
</html>)")
      .arg(text, url);
  m_p->m_description->setText(description);
  m_p->m_lastTerm =
    index.sibling(index.row(), static_cast<int>(qtOntologyModel::Column::Name)).data().toString();
}

void qtOntologyItem::modelEntryChosen(const QModelIndex& index)
{
  this->modelEntryHighlighted(index);
  // Set attribute items' data to this index, emitting a signal
  // if any item's value is modified.
  auto nameValue = index.sibling(index.row(), static_cast<int>(qtOntologyModel::Column::Name))
                     .data()
                     .toString()
                     .toStdString();

  if (nameValue.empty())
  {
    // An empty string is an invalid name.
    return;
  }

  if (!m_p->addTermToTagList(index))
  {
    // The term has already been queued.
    return;
  }

  // We know we need to add an entry to the group since we added it to the table-widget.
  auto addGroupItem = m_itemInfo.itemAs<GroupItem>();
  std::size_t tt = addGroupItem->numberOfGroups();
  if (!addGroupItem->appendGroup())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Unable to insert another tag to \"tagsToAdd\" group.");
    return;
  }
  auto urlValue = index.sibling(index.row(), static_cast<int>(qtOntologyModel::Column::URL))
                    .data()
                    .toString()
                    .toStdString();
  addGroupItem->findAs<StringItem>(tt, "name")->setValue(nameValue);
  addGroupItem->findAs<StringItem>(tt, "url")->setValue(urlValue);
  addGroupItem->findAs<StringItem>(tt, "ontology")->setValue(m_p->m_ontologyName);
  Q_EMIT modified();
  if (auto* iview = m_itemInfo.baseView())
  {
    iview->valueChanged(addGroupItem);
  }
}

void qtOntologyItem::textActivated(const QString& text)
{
  if (!m_p->m_model)
  {
    return;
  }

  if (text.isEmpty())
  {
    m_p->m_description->setText("");
  }

  auto startIndex =
    m_p->m_model->index(0, static_cast<int>(qtOntologyModel::Column::Name), QModelIndex());
  auto matches =
    m_p->m_model->match(startIndex, Qt::DisplayRole, text, 1, Qt::MatchFlags(Qt::MatchExactly));
  if (!matches.empty())
  {
    this->modelEntryChosen(matches[0]);
  }
}

void qtOntologyItem::emptyDescriptionOnEmptySearch(const QString& search)
{
  if (search.isEmpty())
  {
    m_p->m_description->setText("");
  }
}

void qtOntologyItem::attemptToAddTag()
{
  this->textActivated(m_p->m_name->currentText());
}

void qtOntologyItem::removeSelectedTags()
{
  auto items = m_p->m_addedTagList->selectedItems();
  if (items.isEmpty())
  {
    return;
  }
  bool didRemove = false;
  auto addGroup = m_itemInfo.itemAs<GroupItem>();
  for (const auto& item : items)
  {
    std::size_t nn = addGroup->numberOfGroups();
    for (std::size_t ii = 0; ii < nn; ++ii)
    {
      if (addGroup->findAs<StringItem>(ii, "name")->value() == item->text().toStdString())
      {
        didRemove = true;
        addGroup->removeGroup(ii);
        // m_p->m_addedTagList->takeItem(item->row(), item->column());
        m_p->m_addedTagList->removeRow(item->row());
        break;
      }
    }
  }
  if (didRemove)
  {
    Q_EMIT modified();
    if (auto* iview = m_itemInfo.baseView())
    {
      iview->valueChanged(addGroup);
    }
  }
}

void qtOntologyItem::updateItemData()
{
  auto item = m_itemInfo.itemAs<GroupItem>();
  std::size_t nn = item->numberOfGroups();
  m_p->m_addedTagList->setRowCount(static_cast<int>(nn));
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    auto nameValue = item->findAs<StringItem>(ii, "name")->value();
    auto qNameValue = QString::fromStdString(nameValue);
    auto* tagNameItem = new QTableWidgetItem(qNameValue);
    tagNameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    tagNameItem->setText(qNameValue);
    m_p->m_addedTagList->setItem(static_cast<int>(ii), 0, tagNameItem);
  }
  this->qtItem::updateItemData();
}

void qtOntologyItem::associationsChanged()
{
  auto item = m_itemInfo.item();
  auto assocs = item->attribute()->associations();

  // Find the set of ontology classes the associated items have been tagged with:
  std::map<smtk::string::Token, const smtk::markup::OntologyIdentifier*> tags;
  std::size_t nn = assocs->numberOfValues();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    if (!assocs->isSet(ii))
    {
      continue;
    }

    auto comp = std::dynamic_pointer_cast<smtk::markup::Component>(assocs->value(ii));
    if (comp)
    {
      comp->ontologyClasses().visit(
        [&tags](const smtk::markup::OntologyIdentifier* oid) { tags[oid->ontologyId()] = oid; });
    }
  }

  // Clear the "Remove" tab's table-widget of rows with no matching tag.
  auto* removeList = m_p->m_removableTagList;
  for (int ii = removeList->rowCount() - 1; ii >= 0; --ii)
  {
    smtk::string::Token rowTag = removeList->item(ii, 1)->text().toStdString();
    auto it = tags.find(rowTag);
    if (it == tags.end())
    {
      // No associated items have this tag. Remove it from the table.
      removeList->removeRow(ii);
    }
    else
    {
      // This tag is already in the table, no need to add it later. Remove it from the tags map.
      tags.erase(it);
    }
  }
  // Populate the "Remove" tab's table-widget with an entry per remaining tag.
  bool didInsert = false;
  for (const auto& entry : tags)
  {
    const auto* tag = entry.second;
    int rowIndex = removeList->rowCount();
    removeList->insertRow(rowIndex);
    auto* ontNameItem = new QTableWidgetItem(QString::fromStdString(tag->name()));
    auto* ontUrlItem = new QTableWidgetItem(QString::fromStdString(tag->ontologyId().data()));
    ontNameItem->setCheckState(Qt::Unchecked);
    ontNameItem->setFlags(
      Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled |
      Qt::ItemNeverHasChildren);
    ontUrlItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    removeList->setItem(rowIndex, 0, ontNameItem);
    removeList->setItem(rowIndex, 1, ontUrlItem);
    didInsert = true;
  }
  if (didInsert)
  {
    // Sort the table so newly-inserted rows appear in the proper position.
    auto sortOrder = removeList->horizontalHeader()->sortIndicatorOrder();
    auto sortColumn = removeList->horizontalHeader()->sortIndicatorSection();
    removeList->sortItems(sortColumn, sortOrder);
  }
}

void qtOntologyItem::associationsMayHaveChanged(const smtk::attribute::Item::Ptr& item)
{
  if (auto refItem = std::dynamic_pointer_cast<ReferenceItem>(item))
  {
    if (refItem->attribute() && refItem->attribute()->associations() == refItem)
    {
      this->associationsChanged();
    }
  }
}

void qtOntologyItem::createWidget()
{
  smtk::attribute::ItemPtr item = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(item))
  {
    return;
  }

  this->clearChildItems();
  this->updateUI();
}

void qtOntologyItem::updateUI()
{
  auto* iview = m_itemInfo.baseView();
  auto item = m_itemInfo.itemAs<GroupItem>();
  auto itemDef = item->definition();
  if (iview && !iview->displayItem(item))
  {
    return;
  }

  if (m_widget)
  {
    delete m_widget;
  }

  m_widget = new QFrame(m_itemInfo.parentWidget());
  m_widget->setObjectName(item->name().c_str());
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  auto* top = new QHBoxLayout(m_widget);
  m_p->m_content = new QWidget(m_widget);
  top->setObjectName("top");

  top->setMargin(0);
  top->setSpacing(0);
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QSizePolicy minExpPolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setObjectName("labelLayout");
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  // Note that the definition could be optional but the item maybe forced
  // to be required.  We need to still create the check box in case
  // the item's force required state is changed
  if (itemDef->isOptional())
  {
    m_p->m_enable = new QCheckBox(m_itemInfo.parentWidget());
    m_p->m_enable->setObjectName("EnableButton");
    m_p->m_enable->setChecked(item->localEnabledState());
    m_p->m_enable->setText(" ");
    m_p->m_enable->setSizePolicy(sizeFixedPolicy);
    padding = m_p->m_enable->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(m_p->m_enable, SIGNAL(stateChanged(int)), this, SLOT(setItemEnabled(int)));
    labelLayout->addWidget(m_p->m_enable);
    if (!item->isEnabled())
    {
      m_p->m_enable->setVisible(false);
      m_p->m_content->setVisible(true);
    }
  }

  QString labelText = item->label().c_str();
  QLabel* label = new QLabel(labelText);
  label->setObjectName("ontologyItemLabel");
  label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    label->setToolTip(strBriefDescription.c_str());
  }

  if (itemDef->advanceLevel() && m_itemInfo.baseView())
  {
    label->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  if (labelText != " ")
  {
    labelLayout->addWidget(label);
  }
  m_p->m_label = label;

  if (label->text().trimmed().isEmpty())
  {
    this->setLabelVisible(false);
  }

  top->addLayout(labelLayout);
  // Create a frame to hold the item's contents
  m_p->m_content->setObjectName("Contents");
  m_p->setupUi(m_p->m_content);
  top->addWidget(m_p->m_content);
  m_p->m_description->setOpenExternalLinks(true);

  m_p->m_completer = new QCompleter(m_p->m_name);
  m_p->m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
  m_p->m_completer->setCompletionMode(QCompleter::PopupCompletion);
  m_p->m_completer->setFilterMode(Qt::MatchContains);
  std::string ontologyName;
  if (m_itemInfo.component().attribute("OntologyModel", ontologyName))
  {
    if (!this->setOntologyModel(ontologyName))
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "No ontology named \"" << ontologyName << "\" available.");
      m_p->m_model = nullptr;
    }
  }
  else
  {
    smtkWarningMacro(smtk::io::Logger::instance(), "No ontology specified in configuration.");
    m_p->m_model = nullptr;
  }
  // When user chooses an identifier, update the labels and item values.
  QObject::connect(
    m_p->m_completer,
    (void (QCompleter::*)(const QModelIndex&)) & QCompleter::activated,
    this,
    &qtOntologyItem::modelEntryChosen);
  // When the user hovers over an identifier, update the labels (but not the item values).
  QObject::connect(
    m_p->m_completer,
    (void (QCompleter::*)(const QModelIndex&)) & QCompleter::highlighted,
    this,
    &qtOntologyItem::modelEntryHighlighted);
  // Clear label text when the user enters a non-match
  QObject::connect(m_p->m_name, &QComboBox::textActivated, this, &qtOntologyItem::textActivated);
#if 1
  // Clear label text when the user enters a non-match
  QObject::connect(
    m_p->m_name, &QComboBox::editTextChanged, this, &qtOntologyItem::emptyDescriptionOnEmptySearch);
#endif

  if (m_itemInfo.parentWidget() && m_itemInfo.parentWidget()->layout())
  {
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  if (item->isOptional())
  {
    this->setItemEnabled(item->localEnabledState() ? 1 : 0);
  }
  m_p->m_name->lineEdit()->setPlaceholderText("Tag name… (e.g., \"femur\")");
  // this->updateItemData();

  // Monitor for changes to the operation's associations so we can update
  // the set of "removable" ontology classes.
  QObject::connect(
    m_itemInfo.baseView(),
    &qtBaseAttributeView::modified,
    this,
    &qtOntologyItem::associationsMayHaveChanged);
  this->associationsChanged();

  // When users click on the plus ("+") button, attempt to add
  // the current text as an ontology class to the table-widget
  // below (m_p->m_addedTagList).
  QObject::connect(
    m_p->m_addTagButton, &QPushButton::clicked, this, &qtOntologyItem::attemptToAddTag);

  // When users click on the minus ("-") button, prune
  // the list of tags to be added by the selection.
  QObject::connect(
    m_p->m_removeTagButton, &QPushButton::clicked, this, &qtOntologyItem::removeSelectedTags);

  // When the selection is empty, disable the minus ("-") button.
  QObject::connect(
    m_p->m_addedTagList->selectionModel(), &QItemSelectionModel::selectionChanged, [&]() {
      bool emptyList = m_p->m_addedTagList->selectedItems().isEmpty();
      m_p->m_removeTagButton->setEnabled(!emptyList);
    });
  m_p->m_removeTagButton->setEnabled(false); // By default, the table is empty.

  /// Observe this operation when it is launched so that
  /// 1. We can update the check-state of all identifiers from the GUI at the time the operation runs
  ///    (this is needed because Qt does not emit signals when item check-states change.
  /// 2. We can reset the GUI when the operation completes so the UI is usable (i.e., tags which were
  ///    just added will not be re-added but instead made available for removal).
  if (auto opMgr = m_itemInfo.baseView()->information().get<smtk::operation::Manager::Ptr>())
  {
    static auto key = opMgr->observers().insert(
      [&](
        const smtk::operation::Operation& op,
        smtk::operation::EventType event,
        smtk::operation::Operation::Result) -> int {
        if (event == smtk::operation::EventType::DID_OPERATE)
        {
          if (const auto* tagOp = dynamic_cast<const smtk::markup::TagIndividual*>(&op))
          {
            if (tagOp->parameters() == m_itemInfo.item()->attribute())
            {
              // We own locks on the operation inputs, so it is OK to reset things now.
              m_itemInfo.itemAs<GroupItem>()->setNumberOfGroups(0);
              tagOp->parameters()->findComponent("tagsToRemove")->setNumberOfValues(0);
              this->associationsChanged();
              this->updateItemData();
            }
          }
        }
        return 0;
      },
      "qtOntologyItem observer");
  }
  if (auto* opView = m_itemInfo.baseView()->information().get<smtk::extension::qtOperationView*>())
  {
    opView->setDisableApplyAfterRun(false);
    QObject::connect(
      opView, &qtOperationView::operationRequested, [&](const smtk::operation::Operation::Ptr& op) {
        auto tagsToRemove = op->parameters()->findComponent("tagsToRemove");
        m_p->copyRemovableTagsToParameter(tagsToRemove);
      });
  }
}

void qtOntologyItem::setItemEnabled(int checkState)
{
  auto item = m_itemInfo.itemAs<StringItem>();
  if (!(item && m_widget))
  {
    return;
  }
  bool enable = checkState != 0;
  m_p->m_content->setVisible(enable);
  if (enable != item->localEnabledState())
  {
    item->setIsEnabled(enable);
    Q_EMIT this->modified();
    auto* iview = m_itemInfo.baseView();
    if (iview)
    {
      iview->valueChanged(item);
    }
  }
}

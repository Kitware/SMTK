//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtReferenceItem.h"
#include "smtk/extension/qt/qtReferenceItemData.h"
#include "smtk/extension/qt/qtReferenceItemEditor.h"

#include "smtk/extension/qt/MembershipBadge.h"
#include "smtk/extension/qt/qtBadgeActionToggle.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/Manager.h"
#include "smtk/view/ReferenceItemPhraseModel.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include "smtk/io/Logger.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QTimer>
#include <QWidgetAction>

using namespace smtk::extension;
using namespace smtk::attribute;
using MembershipBadge = smtk::extension::qt::MembershipBadge;

namespace
{
void updateLabel(QLabel* lbl, const QString& txt, bool ok)
{
  lbl->setText(txt);
  lbl->setAutoFillBackground(!ok);
  QPalette pal = lbl->palette();
  pal.setColor(QPalette::Window, QColor(QRgb(ok ? 0x00ff00 : 0xff7777)));
  lbl->setPalette(pal);
  lbl->update();
}

nlohmann::json defaultConfiguration = {
  { "Name", "RefItem" },
  { "Type", "smtk::view::ReferenceItemPhraseModel" },
  { "Component",
    { { "Name", "Details" },
      { "Attributes", { { "TopLevel", true }, { "Title", "Resources" } } },
      { "Children",
        { { { "Name", "PhraseModel" },
            { "Attributes", { { "Type", "smtk::view::ReferenceItemPhraseModel" } } },
            { "Children",
              { { { "Name", "SubphraseGenerator" }, { "Attributes", { { "Type", "none" } } } },
                { { "Name", "Badges" },
                  { "Children",
                    { { { "Name", "Badge" },
                        { "Attributes",
                          { { "Default", true },
                            { "Type",
                              "smtk::extension::qt::MembershipBadge" } } } } } } } } } } } } } }
};
} // namespace

qtItem* qtReferenceItem::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  auto item = info.itemAs<smtk::attribute::ReferenceItem>();
  if (item == nullptr)
  {
    return nullptr;
  }
  // If we are dealing with a non-extensible item with only 1 required value lets
  // use a simple combobox UI else we will use the more advance UI.
  auto itemDef = item->definitionAs<smtk::attribute::ReferenceItemDefinition>();
  if ((itemDef->numberOfRequiredValues() == 1) && !itemDef->isExtensible())
  {
    return new qtReferenceItemEditor(info);
  }
  auto* qi = new qtReferenceItem(info);
  // Unlike its subclasses, qtReferenceItem does not call
  // createWidget in its constructor (because that would cause
  // problems for subclasses since the method is virtual and
  // could be overridden). Call it now:
  qi->createWidget();
  return qi;
}

qtReferenceItemData::qtReferenceItemData()
  : m_selectedIconURL(":/icons/display/selected.png")
  , m_unselectedIconURL(":/icons/display/unselected.png")
{
}

qtReferenceItemData::~qtReferenceItemData() = default;

qtReferenceItem::qtReferenceItem(const qtAttributeItemInfo& info)
  : Superclass(info)
  , m_p(new qtReferenceItemData)
{
  // Grab default icons from the configuration object if specified.
  std::string selectedIconURL;
  std::string unselectedIconURL;
  if (
    info.component().attribute("ItemMemberIcon", selectedIconURL) &&
    info.component().attribute("ItemNonMemberIcon", unselectedIconURL))
  {
    this->setSelectionIconPaths(selectedIconURL, unselectedIconURL);
  }
}

qtReferenceItem::~qtReferenceItem()
{
  this->removeObservers();
  delete m_p;
  m_p = nullptr;
}

void qtReferenceItem::markForDeletion()
{
  this->removeObservers();
  this->qtItem::markForDeletion();
}

void qtReferenceItem::removeObservers()
{
  if (m_p->m_phraseModel && m_p->m_modelObserverId.assigned())
  {
    m_p->m_phraseModel->observers().erase(m_p->m_modelObserverId);
  }
  QObject::disconnect(this);
}

qtReferenceItem::AcceptsTypes qtReferenceItem::acceptableTypes() const
{
  auto def = std::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(
    m_itemInfo.item()->definition());
  if (!def)
  {
    return NONE;
  }

  bool rsrc = false;
  bool comp = false;
  for (const auto& entry : def->acceptableEntries())
  {
    if (entry.second.empty())
    {
      rsrc = true;
    }
    else
    {
      comp = true;
    }
  }
  return rsrc ? (comp ? BOTH : RESOURCES) : (comp ? COMPONENTS : NONE);
}

void qtReferenceItem::setLabelVisible(bool visible)
{
  m_p->m_label->setVisible(visible);
}

bool qtReferenceItem::setSelectionIconPaths(
  const std::string& selectedIconPath,
  const std::string& unselectedIconPath)
{
  if (m_p->m_selectedIconURL == selectedIconPath && m_p->m_unselectedIconURL == unselectedIconPath)
  {
    return false;
  }

  m_p->m_selectedIconURL = selectedIconPath;
  m_p->m_unselectedIconURL = unselectedIconPath;
  if (m_p->m_qtModel)
  {
    m_p->m_qtModel->setVisibleIconURL(m_p->m_selectedIconURL);
    m_p->m_qtModel->setInvisibleIconURL(m_p->m_unselectedIconURL);
  }
  return true;
}

std::pair<std::string, std::string> qtReferenceItem::selectionIconPaths() const
{
  return std::make_pair(m_p->m_selectedIconURL, m_p->m_unselectedIconURL);
}

void qtReferenceItem::updateItemData()
{
  this->updateUI();
  this->Superclass::updateItemData();
}

void qtReferenceItem::selectionLinkToggled(bool linked)
{
  (void)linked;
}

void qtReferenceItem::setOutputOptional(int state)
{
  auto itm = m_itemInfo.item();
  bool optionalVal = (state != 0);
  // Lets set the item 's optional state if it is different
  if (itm && (itm->localEnabledState() != optionalVal))
  {
    itm->setIsEnabled(optionalVal);
    Q_EMIT modified();
  }
  m_p->m_editBtn->setEnabled(optionalVal);
  this->updateSynopsisLabels();
}

void qtReferenceItem::linkHover(bool link)
{
  auto seln = this->uiManager() ? this->uiManager()->selection() : nullptr;
  if (!seln)
  {
    return;
  }

  smtk::resource::PersistentObjectArray hover;
  if (link)
  {
    // Traverse entries of m_itemInfo.item() and ensure their "hover" bit is set
    // in the application selection.
    for (const auto& member : this->members())
    {
      if (member.second)
      {
        if (auto object = member.first.lock())
        {
          hover.push_back(object);
        }
      }
    }
  } // else the mouse is no longer hovering... clear the highlight.
  // std::cout << "Hover " << (link ? "link" : "unlink") << " " << hover.size() << " items" << "\n";
  seln->modifySelection(
    hover, "qtReferenceItemHover", 0x02, smtk::view::SelectionAction::UNFILTERED_REPLACE, true);
  /*
  // TODO: traverse entries of m_itemInfo.item() and ensure their "hover" bit is set
  //       in the application selection.
    */
}

void qtReferenceItem::linkHoverTrue()
{
  this->linkHover(true);
}

void qtReferenceItem::linkHoverFalse()
{
  this->linkHover(false);
}

void qtReferenceItem::synchronizeAndHide(bool escaping)
{
  bool ok;
  std::string syn;
  if (!escaping)
  {
    syn = this->synopsis(ok);
  }
  else
  {
    ok = false;
  }

  if (!ok || !(ok = this->synchronize(UpdateSource::ITEM_FROM_GUI)))
  {
    // We cannot hide if the state is invalid...
    // revert to the item's current state.
    // That state may still be invalid (e.g., because
    // the item has required, non-default values).
    // But what else can we do?
    ok = this->synchronize(UpdateSource::GUI_FROM_ITEM);
    syn = this->synopsis(ok);
  }

  QString qsyn = QString::fromStdString(syn);
  updateLabel(m_p->m_synopsis, qsyn, ok);

  if (!m_p->m_alreadyClosingPopup)
  {
    m_p->m_alreadyClosingPopup = true;
    m_p->m_editBtn->menu()->hide();
    m_p->m_alreadyClosingPopup = false;
  }
}

void qtReferenceItem::copyFromSelection()
{
  if (!m_itemInfo.uiManager())
  {
    return;
  }
  auto seln = m_itemInfo.uiManager()->selection();
  if (seln)
  {
    auto selnSet = seln->currentSelectionByValueAs<smtk::resource::PersistentObjectArray>(1);
    if (m_itemInfo.itemAs<smtk::attribute::ReferenceItem>()->setValues(
          selnSet.begin(), selnSet.end()))
    {
      if (this->synchronize(UpdateSource::GUI_FROM_ITEM))
      {
        this->updateSynopsisLabels();
        this->linkHover(true);
        Q_EMIT modified();
      }
    }
  }
}

void qtReferenceItem::copyToSelection()
{
  if (!m_itemInfo.uiManager())
  {
    return;
  }
  auto seln = m_itemInfo.uiManager()->selection();
  if (seln)
  {
    smtk::resource::PersistentObjectArray nextSeln;
    for (const auto& entry : this->members())
    {
      if (auto object = entry.first.lock())
      {
        nextSeln.push_back(entry.first.lock());
      }
    }
    seln->modifySelection(nextSeln, "qtReferenceItem", 1); // FIXME: Use an app-specified bit
  }
}

void qtReferenceItem::clearItem()
{
  m_itemInfo.item()->reset();
  if (this->synchronize(UpdateSource::GUI_FROM_ITEM))
  {
    this->updateSynopsisLabels();
    this->linkHover(true);
    Q_EMIT modified();
  }
}

void qtReferenceItem::sneakilyHideButtons()
{
  m_p->m_copyFromSelection->setVisible(false);
  m_p->m_clear->setVisible(false);
  m_p->m_copyToSelection->setVisible(false);
}

void qtReferenceItem::cleverlyShowButtons()
{
  m_p->m_copyFromSelection->setVisible(true);
  m_p->m_clear->setVisible(true);
  m_p->m_copyToSelection->setVisible(true);
}

smtk::view::PhraseModelPtr qtReferenceItem::createPhraseModel() const
{
  // Constructing the PhraseModel with a factory from our config, that includes
  // the MembershipBadge.
  smtk::view::ConfigurationPtr phraseModelConfig;
  auto refItem = std::dynamic_pointer_cast<smtk::attribute::ReferenceItem>(m_itemInfo.item());
  // If the item only allows single selection, configure the membership badge for it.
  if (
    refItem && refItem->numberOfRequiredValues() < 2 &&
    ((!refItem->isExtensible()) || (refItem->isExtensible() && refItem->maxNumberOfValues() == 1)))
  {
    json jj = defaultConfiguration;
    jj["Component"]["Children"][0]["Children"][1]["Children"][0]["Attributes"]["SingleSelect"] =
      true;
    phraseModelConfig = jj;
  }
  else
  {
    phraseModelConfig = defaultConfiguration;
  }
  auto phraseModel =
    m_itemInfo.uiManager()->viewManager()->phraseModelFactory().createFromConfiguration(
      phraseModelConfig.get());
  auto def = std::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(
    m_itemInfo.item()->definition());
  std::static_pointer_cast<smtk::view::ReferenceItemPhraseModel>(phraseModel)
    ->setReferenceItem(refItem);
  phraseModel->addSource(m_itemInfo.uiManager()->managers());
  return phraseModel;
}

void qtReferenceItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }

  this->clearWidgets();
  this->updateItemData();
}

void qtReferenceItem::clearWidgets()
{
  auto pwidget = m_itemInfo.parentWidget();
  if (!pwidget)
  {
    return;
  }
  pwidget->layout()->removeWidget(m_widget);
  delete m_widget;
  m_widget = nullptr;
}

void qtReferenceItem::updateUI()
{
  smtk::attribute::ItemPtr itm = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(itm))
  {
    return;
  }

  // TODO: this need to connect to the right managers

  auto phraseModel = this->createPhraseModel();
  m_p->m_phraseModel = phraseModel;
  m_p->m_qtModel = new qtDescriptivePhraseModel;
  m_p->m_qtModel->setPhraseModel(m_p->m_phraseModel);
  m_p->m_qtDelegate = new smtk::extension::qtDescriptivePhraseDelegate;
  m_p->m_qtDelegate->setTextVerticalPad(6);
  m_p->m_qtDelegate->setTitleFontWeight(1);
  m_p->m_qtDelegate->setDrawSubtitle(false);
  m_p->m_qtDelegate->setVisibilityMode(true);

  if (m_p->m_phraseModel)
  {
    m_p->m_phraseModel->addSource(m_itemInfo.uiManager()->managers());
    QPointer<qtReferenceItem> guardedObject(this);
    m_p->m_modelObserverId = m_p->m_phraseModel->observers().insert(
      [guardedObject](
        smtk::view::DescriptivePhrasePtr phr,
        smtk::view::PhraseModelEvent evt,
        const std::vector<int>& src,
        const std::vector<int>& dst,
        const std::vector<int>& refs) {
        if (guardedObject)
        {
          guardedObject->checkRemovedComponents(phr, evt, src, dst, refs);
        }
      },
      "qtReferenceItem: Check for removed components.");
    // we need to know when membership is changed, to update our labels
    MembershipBadge* badge =
      m_p->m_phraseModel->badges().findBadgeOfType<smtk::extension::qt::MembershipBadge>();
    QObject::connect(
      badge, &MembershipBadge::membershipChange, this, &qtReferenceItem::membershipChanged);
  }

  // Create a container for the item:
  if (m_widget)
  {
    delete m_widget;
  }

  m_widget = new QFrame(m_itemInfo.parentWidget());
  m_widget->setObjectName("ReferenceItemFrame");
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  m_widget->installEventFilter(this);
  m_p->m_grid = new QGridLayout(m_widget);
  m_p->m_grid->setObjectName("grid");
  m_p->m_grid->setMargin(0);
  m_p->m_grid->setSpacing(0);
  m_p->m_grid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QSizePolicy sizeStretchyXPolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

  // Create a layout for the item's checkbox (if it is optional) and its label.
  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setObjectName("labelLayout");
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // Add the "enable" checkbox if the item is optional.
  int padding = 0;
  if (itm->isOptional())
  {
    m_p->m_optional = new QCheckBox(m_itemInfo.parentWidget());
    m_p->m_optional->setObjectName("optionalCheckbox");
    m_p->m_optional->setChecked(itm->localEnabledState());
    m_p->m_optional->setText(" ");
    m_p->m_optional->setSizePolicy(sizeFixedPolicy);
    padding = m_p->m_optional->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(
      m_p->m_optional, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(m_p->m_optional);
  }
  else
  {
    m_p->m_optional = nullptr;
  }
  auto itemDef = itm->definition();

  // Add a label for the item.
  QString labelText = !itm->label().empty() ? itm->label().c_str() : itm->name().c_str();
  m_p->m_label = new QLabel(labelText, m_widget);
  m_p->m_label->setObjectName("label");
  m_p->m_label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    m_p->m_label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  m_p->m_label->setWordWrap(true);
  m_p->m_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // Add in BriefDescription as tooltip if available:
  const std::string strBriefDescription = itemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    m_p->m_label->setToolTip(strBriefDescription.c_str());
  }

  if (itemDef->advanceLevel() && m_itemInfo.uiManager())
  {
    m_p->m_label->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  labelLayout->addWidget(m_p->m_label);

  // Now add widgetry for the "entry"
  // Create a layout for the item's entry editor.
  QHBoxLayout* entryLayout = new QHBoxLayout();
  entryLayout->setObjectName("ReferenceItemLayout");
  entryLayout->setMargin(0);
  entryLayout->setSpacing(6);
  entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // An entry consists of ...
  // ... a button to grab the selection
  QIcon copyFromSelection(":/icons/reference-item/copy-from-selection.png");
  m_p->m_copyFromSelection = new QPushButton(copyFromSelection, "");
  m_p->m_copyFromSelection->setObjectName("CopyFromSelection");
  m_p->m_copyFromSelection->setSizePolicy(sizeFixedPolicy);
  m_p->m_copyFromSelection->setToolTip("Replace this item's members with the selection.");
  entryLayout->addWidget(m_p->m_copyFromSelection);
  QObject::connect(m_p->m_copyFromSelection, SIGNAL(clicked()), this, SLOT(copyFromSelection()));

  // ... a button to empty the item's members
  QIcon clearItem(":/icons/reference-item/clear.png");
  m_p->m_clear = new QPushButton(clearItem, "");
  m_p->m_clear->setObjectName("ClearMembership");
  m_p->m_clear->setSizePolicy(sizeFixedPolicy);
  m_p->m_clear->setToolTip("Clear this item's members.");
  entryLayout->addWidget(m_p->m_clear);
  QObject::connect(m_p->m_clear, SIGNAL(clicked()), this, SLOT(clearItem()));

  // ... a button to populate the selection with the item's members
  QIcon copyToSelection(":/icons/reference-item/copy-to-selection.png");
  m_p->m_copyToSelection = new QPushButton(copyToSelection, "");
  m_p->m_copyToSelection->setObjectName("CopyToSelection");
  m_p->m_copyToSelection->setSizePolicy(sizeFixedPolicy);
  m_p->m_copyToSelection->setToolTip("Replace the selection with this item's members.");
  entryLayout->addWidget(m_p->m_copyToSelection);
  QObject::connect(m_p->m_copyToSelection, SIGNAL(clicked()), this, SLOT(copyToSelection()));

  // ... a synopsis (label).
  bool ok;
  QString synText = QString::fromStdString(this->synopsis(ok));
  m_p->m_synopsis = new QLabel(synText, m_widget);
  m_p->m_synopsis->setObjectName("synopsis");
  m_p->m_synopsis->setSizePolicy(sizeStretchyXPolicy);
  m_p->m_synopsis->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  entryLayout->addWidget(m_p->m_synopsis);

  // ... a button to pop up an editor for the item contents.
  m_p->m_editBtn = new QToolButton(m_widget);
  m_p->m_editBtn->setObjectName("EditReferenceItemMembers");
  m_p->m_editBtn->setPopupMode(QToolButton::InstantPopup);
  m_p->m_editBtn->setMenu(new QMenu(m_p->m_editBtn));
  m_p->m_editBtn->menu()->setObjectName("Candidates");
  entryLayout->addWidget(m_p->m_editBtn);

  // Create a popup for editing the item's contents
  auto refItem = m_itemInfo.itemAs<smtk::attribute::ReferenceItem>();
  bool multiselect = refItem &&
    (refItem->numberOfRequiredValues() > 1 ||
     (refItem->isExtensible() && refItem->maxNumberOfValues() != 1));
  m_p->m_popup = new QDialog(m_p->m_editBtn);
  m_p->m_popup->setObjectName("popup");
  m_p->m_popupLayout = new QVBoxLayout(m_p->m_popup);
  m_p->m_popupLayout->setObjectName("popupLayout");
  m_p->m_popupList = new QListView(m_p->m_popup);
  m_p->m_popupList->setObjectName("popupList");
  m_p->m_popupList->setItemDelegate(m_p->m_qtDelegate);
  m_p->m_popupLayout->addWidget(m_p->m_popupList);
  m_p->m_popup->installEventFilter(this);
  m_p->m_popupList->viewport()->installEventFilter(this);
  m_p->m_popupList->setModel(m_p->m_qtModel);
  m_p->m_popupList->setSelectionMode(
    multiselect ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
  m_p->m_popupList->setSelectionBehavior(QAbstractItemView::SelectRows);
  auto* action = new QWidgetAction(m_p->m_editBtn);
  action->setObjectName("action");
  action->setDefaultWidget(m_p->m_popup);
  m_p->m_editBtn->menu()->addAction(action);
  m_p->m_editBtn->setMaximumSize(QSize(16, 20));

  QObject::connect(m_p->m_editBtn->menu(), SIGNAL(aboutToHide()), this, SLOT(popupClosing()));
  // QObject::connect(m_p->m_qtDelegate, SIGNAL(requestVisibilityChange(const QModelIndex&)),
  //   m_p->m_qtModel, SLOT(toggleVisibility(const QModelIndex&)));

  m_p->m_grid->addLayout(labelLayout, 0, 0);
  m_p->m_grid->addLayout(entryLayout, 0, 1);

  if (m_itemInfo.parentWidget() && m_itemInfo.parentWidget()->layout())
  {
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  if (itm->isOptional())
  {
    this->setOutputOptional(itm->localEnabledState() ? 1 : 0);
  }
  this->synchronize(UpdateSource::GUI_FROM_ITEM);

  // Add a vertical spacer the same height as buttons that are sometimes hidden.
  m_widget->show();
  entryLayout->addItem(new QSpacerItem(
    0,
    entryLayout->geometry().height() + entryLayout->spacing(),
    QSizePolicy::Fixed,
    QSizePolicy::Fixed));

  this->sneakilyHideButtons();
  this->updateSynopsisLabels();
}

void qtReferenceItem::popupClosing()
{
  if (!m_p->m_alreadyClosingPopup)
  {
    m_p->m_alreadyClosingPopup = true;
    this->synchronizeAndHide(false);
    m_p->m_alreadyClosingPopup = false;
  }
}

std::string qtReferenceItem::synopsis(bool& ok) const
{
  auto item = m_itemInfo.itemAs<smtk::attribute::ReferenceItem>();
  if (!item)
  {
    ok = false;
    return "uninitialized item";
  }

  std::size_t numRequired = item->numberOfRequiredValues();
  std::size_t maxAllowed = (item->isExtensible() ? item->maxNumberOfValues() : numRequired);
  std::ostringstream label;
  std::size_t numSel = 0;
  for (const auto& entry : this->members())
  {
    if (entry.second > 0)
    {
      ++numSel;
    }
  }
  ok = true;
  if (numRequired < 2 && maxAllowed == 1)
  {
    auto ment =
      (this->members().empty() ? smtk::resource::PersistentObjectPtr()
                               : this->members().begin()->first.lock());
    label
      << (numSel == 1 ? (ment ? ment->name() : "NULL!!") : (numSel > 0 ? "too many" : "(none)"));
    ok = numSel >= numRequired && numSel <= maxAllowed;
  }
  else
  {
    label << numSel;
    if (numRequired > 0)
    {
      label << " of ";
      if (numRequired == maxAllowed)
      { // Exactly N values are allowed and required.
        label << numRequired;
      }
      else if (maxAllowed > 0)
      { // There is a minimum required, but a limited additional number are acceptable
        label << numRequired << "—" << maxAllowed;
      }
      else
      { // Any number are allowed, but there is a minimum.
        label << numRequired << "+";
      }
      ok &= (numSel >= numRequired);
    }
    else
    { // no values are required, but there may be a cap on the maximum number.
      if (maxAllowed > 0)
      {
        label << " of 0–" << maxAllowed;
      }
      else
      {
        label << " chosen";
      }
    }
  }
  ok &= (maxAllowed == 0 || numSel <= maxAllowed);
  return label.str();
}

void qtReferenceItem::updateSynopsisLabels() const
{
  if (!m_p || !m_p->m_synopsis)
  {
    return;
  }

  bool ok = true;
  std::string syn = m_p->m_optional && !m_p->m_optional->isChecked() ? std::string("(disabled)")
                                                                     : this->synopsis(ok);
  QString qsyn = QString::fromStdString(syn);
  updateLabel(m_p->m_synopsis, qsyn, ok);
}

bool qtReferenceItem::eventFilter(QObject* src, QEvent* event)
{
  // We serve as an event filter on 2 widgets:
  // 1. the inherited frame holding the item (m_widget),
  //    which we monitor for the enter/exit events that enable hovering
  // 2. the popup dialog (m_p->m_popup)
  //    which we monitor for keyboard navigation events
  if (src == m_widget)
  {
    switch (event->type())
    {
      case QEvent::Enter:
        QTimer::singleShot(0, this, SLOT(linkHoverTrue()));
        this->cleverlyShowButtons();
        break;
      case QEvent::Leave:
        QTimer::singleShot(0, this, SLOT(linkHoverFalse()));
        this->sneakilyHideButtons();
        break;
      case QEvent::FocusIn:
        this->cleverlyShowButtons();
        QTimer::singleShot(0, this, SLOT(linkHoverTrue()));
        break;
      case QEvent::FocusOut:
        QTimer::singleShot(0, this, SLOT(linkHoverFalse()));
        this->sneakilyHideButtons();
        break;
      default:
        break;
    }
  }
  else if (src == m_p->m_popup)
  {
    // std::cout << "Popup event: " << event->type() << "\n";
    switch (event->type())
    {
      case QEvent::KeyPress:
      case QEvent::ShortcutOverride: // What keypresses look like to the parent of the QListView.
      {
        // std::cout << "  Popup key\n";
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        int kk = keyEvent->key();
        switch (kk)
        {
          case Qt::Key_Escape:
          case Qt::Key_Cancel:
            // std::cout << "    Hiding\n";
            this->synchronizeAndHide(true);
            return true;
            break;
          case Qt::Key_Return:
          case Qt::Key_Enter:
            this->synchronizeAndHide(false);
            return true;
            break;
          case Qt::Key_Space:
            // std::cout << "    Toggling\n";
            this->toggleCurrentItem();
            return true;
            break;
          default:
            break;
        }
      }
      break;
      case QEvent::Hide:
      {
        if (m_p->m_popup->isVisible())
        {
          // The user has clicked outside the popup.
          // Decide whether to update the item state or abandon.
          this->synchronizeAndHide(false);
          return true;
        }
      }
      break;
      default:
        break;
    }
  }
  else if (
    event->type() == QEvent::MouseButtonPress && m_p->m_popupList->isVisible() &&
    src == m_p->m_popupList->viewport())
  {
    if (qtDescriptivePhraseDelegate::processBadgeClick(
          static_cast<QMouseEvent*>(event), m_p->m_popupList))
    {
      // Consume the click to prevent m_p->m_popupList's selection from changing:
      return true;
    }
  }
  return false; // QObject::eventFilter(src, event);
}

void qtReferenceItem::toggleCurrentItem()
{
  auto cphr = m_p->m_popupList->currentIndex()
                .data(smtk::extension::qtDescriptivePhraseModel::PhrasePtrRole)
                .value<smtk::view::DescriptivePhrasePtr>();
  if (cphr)
  {
    auto persistentObj = cphr->relatedObject();
    auto* badge =
      m_p->m_phraseModel->badges().findBadgeOfType<smtk::extension::qt::MembershipBadge>();
    auto selected = m_p->m_popupList->selectionModel()->selection();
    badge->action(cphr.get(), smtk::extension::qtBadgeActionToggle(selected));
    this->updateSynopsisLabels();
  }
}

void qtReferenceItem::membershipChanged(int)
{
  m_p->m_phraseModel->triggerDataChanged(); // Any change in membership should cause a redraw.
  this->updateSynopsisLabels();
  this->linkHoverTrue();
}

void qtReferenceItem::checkRemovedComponents(
  smtk::view::DescriptivePhrasePtr phr,
  smtk::view::PhraseModelEvent evt,
  const std::vector<int>& src,
  const std::vector<int>& dst,
  const std::vector<int>& refs)
{
  (void)phr;
  (void)dst;
  if (evt == smtk::view::PhraseModelEvent::ABOUT_TO_REMOVE)
  {
    bool didChange = false;
    auto itm = this->itemAs<smtk::attribute::ReferenceItem>();
    // If the application releases its hold on the attribute
    // resource being represented, then we may not have an item:
    if (!itm)
    {
      return;
    }

    auto qidx = m_p->m_qtModel->indexFromPath(src);
    for (auto ref : refs)
    {
      auto ridx = m_p->m_qtModel->index(ref, 0, qidx);
      auto rphr = ridx.data(smtk::extension::qtDescriptivePhraseModel::PhrasePtrRole)
                    .value<smtk::view::DescriptivePhrasePtr>();
      auto comp = rphr ? rphr->relatedComponent() : nullptr;
      auto rsrc = rphr ? rphr->relatedResource() : nullptr;
      if (comp && this->members().find(comp) != this->members().end())
      {
        this->members().erase(comp);
        itm->removeValue(itm->find(comp));
        didChange = true;
      }
      else if (rsrc && this->members().find(rsrc) != this->members().end())
      {
        this->members().erase(rsrc);
        itm->removeValue(itm->find(rsrc));
        didChange = true;
      }
    }
    if (didChange)
    {
      this->updateSynopsisLabels();
    }
  }
}

bool qtReferenceItem::synchronize(UpdateSource src)
{
  auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
  if (!item)
  {
    return false;
  }

  std::size_t uiMembers = 0;
  for (const auto& member : this->members())
  {
    if (member.second)
    {
      ++uiMembers;
    }
  }
  switch (src)
  {
    case UpdateSource::ITEM_FROM_GUI:
    {
      // Everything else in this case statement should really be
      // a single, atomic operation executed on the attribute/item:
      if (!item->setNumberOfValues(uiMembers))
      {
        return false;
      }
      int idx = 0;
      for (const auto& member : this->members())
      {
        if (member.second)
        {
          if (!item->setValue(idx, member.first.lock()))
          {
            std::cerr << "qtReferenceItem: Failed to add " << member.first.lock()->name()
                      << std::endl;
            return false; // Huh!?!
          }
          ++idx;
        }
      }
      Q_EMIT modified();
    }
    break;

    case UpdateSource::GUI_FROM_ITEM:
      this->members().clear();
      m_p->m_phraseModel->triggerDataChanged();
      for (auto vit = item->begin(); vit != item->end(); ++vit)
      {
        // Only allow non-null pointers into the set of selected items;
        // null pointers indicate that the item's entry is invalid and
        // the size of m_members is used to determine whether the
        // association's rules are met, so an extra entry can prevent
        // the association from being edited by the user.
        if (vit.isSet())
        {
          this->members()[*vit] = 1; // FIXME: Use a bit specified by the application.
        }
      }
      break;
  }
  return true;
}

smtk::extension::qt::MembershipBadge::MemberMap& qtReferenceItem::members() const
{
  // We expect there to be only one badge, a MembershipBadge, on our phraseModel
  return (m_p->m_phraseModel->badges().findBadgeOfType<smtk::extension::qt::MembershipBadge>())
    ->getMemberMap();
}

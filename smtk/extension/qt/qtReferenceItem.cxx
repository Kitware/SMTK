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

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include "smtk/io/Logger.h"

#include <QEvent>
#include <QKeyEvent>
#include <QTimer>

using namespace smtk::extension;
using namespace smtk::attribute;

namespace
{
static void updateLabel(QLabel* lbl, const QString& txt, bool ok)
{
  lbl->setText(txt);
  lbl->setAutoFillBackground(ok ? false : true);
  QPalette pal = lbl->palette();
  pal.setColor(QPalette::Background, QColor(QRgb(ok ? 0x00ff00 : 0xff7777)));
  lbl->setPalette(pal);
  lbl->update();
}
}

qtItem* qtReferenceItem::createItemWidget(const AttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::ReferenceItem>() == nullptr)
  {
    return nullptr;
  }
  auto qi = new qtReferenceItem(info);
  // Unlike its subclasses, qtReferenceItem does not call
  // createWidget in its constructor (because that would cause
  // problems for subclasses since the method is virtual and
  // could be overridden). Call it now:
  qi->createWidget();
  return qi;
}

qtReferenceItemData::qtReferenceItemData()
  : m_optional(nullptr)
  , m_modelObserverId(-1)
{
}

qtReferenceItemData::~qtReferenceItemData()
{
}

qtReferenceItem::qtReferenceItem(const AttributeItemInfo& info)
  : Superclass(info)
  , m_p(new qtReferenceItemData)
{
}

qtReferenceItem::~qtReferenceItem()
{
  if (m_p->m_phraseModel && m_p->m_modelObserverId >= 0)
  {
    m_p->m_phraseModel->unobserve(m_p->m_modelObserverId);
  }
  m_p->m_phraseModel->setDecorator([](smtk::view::DescriptivePhrasePtr) {});
  delete m_p;
  m_p = nullptr;
}

qtReferenceItem::AcceptsTypes qtReferenceItem::acceptableTypes() const
{
  auto def = std::dynamic_pointer_cast<const smtk::attribute::ComponentItemDefinition>(
    m_itemInfo.item()->definition());
  if (!def)
  {
    return NONE;
  }

  bool rsrc = false;
  bool comp = false;
  for (auto entry : def->acceptableEntries())
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
  if (itm)
  {
    itm->setIsEnabled(state ? true : false);
  }
  m_p->m_editBtn->setEnabled(state ? true : false);
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
    for (auto member : m_p->m_members)
    {
      if (member.second)
      {
        hover.push_back(member.first);
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
  updateLabel(m_p->m_popupSynopsis, qsyn, ok);

  m_p->m_popup->hide();
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
    if (m_itemInfo.itemAs<smtk::attribute::ReferenceItem>()->setObjectValues(
          selnSet.begin(), selnSet.end()))
    {
      this->synchronize(UpdateSource::GUI_FROM_ITEM);
      this->updateSynopsisLabels();
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
    nextSeln.reserve(m_p->m_members.size());
    for (const auto& entry : m_p->m_members)
    {
      nextSeln.push_back(entry.first);
    }
    seln->modifySelection(nextSeln, "qtReferenceItem", 1); // FIXME: Use an app-specified bit
  }
}

void qtReferenceItem::clearItem()
{
  m_itemInfo.item()->reset();
  this->synchronize(UpdateSource::GUI_FROM_ITEM);
  this->updateSynopsisLabels();
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
  auto showsWhat = this->acceptableTypes();
  switch (showsWhat)
  {
    case COMPONENTS:
    case BOTH:
    case NONE: // Ideally this would do something different.
    {
      // Constructing the PhraseModel with a View properly initializes the SubphraseGenerator
      // to point back to the model (thus ensuring subphrases are decorated). This is required
      // since we need to decorate phrases to show+edit "visibility" as set membership:
      auto phraseModel = smtk::view::ComponentPhraseModel::create(m_itemInfo.component());
      phraseModel->root()->findDelegate()->setModel(phraseModel);
      auto def = std::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(
        m_itemInfo.item()->definition());
      std::static_pointer_cast<smtk::view::ComponentPhraseModel>(phraseModel)
        ->setComponentFilters(def->acceptableEntries());
      return phraseModel;
    }
    break;
    case RESOURCES:
    {
      auto phraseModel = smtk::view::ResourcePhraseModel::create(m_itemInfo.component());
      phraseModel->root()->findDelegate()->setModel(phraseModel);
      auto def = std::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(
        m_itemInfo.item()->definition());
      std::static_pointer_cast<smtk::view::ResourcePhraseModel>(phraseModel)
        ->setResourceFilters(def->acceptableEntries());
      return phraseModel;
    }
    break; // handled below.
  }
  smtkWarningMacro(
    smtk::io::Logger::instance(), "No resources or components accepted. Unsupported.");
  return nullptr;
}

void qtReferenceItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  if (!dataObj || !this->passAdvancedCheck() ||
    (m_itemInfo.uiManager() &&
      !m_itemInfo.uiManager()->passItemCategoryCheck(dataObj->definition())))
  {
    return;
  }

  this->clearWidgets();
  this->updateItemData();
}

void qtReferenceItem::clearWidgets()
{
  m_itemInfo.parentWidget()->layout()->removeWidget(m_widget);
  delete m_widget;
  m_widget = nullptr;
}

void qtReferenceItem::updateUI()
{
  smtk::attribute::ItemPtr itm = m_itemInfo.item();
  if (!itm || !this->passAdvancedCheck() ||
    (m_itemInfo.uiManager() && !m_itemInfo.uiManager()->passItemCategoryCheck(itm->definition())))
  {
    return;
  }

  // TODO: this need to connect to the right managers
  auto rsrcMgr = m_itemInfo.uiManager()->resourceManager();
  auto operMgr = smtk::operation::Manager::create();

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
    m_p->m_phraseModel->setDecorator(
      [this](smtk::view::DescriptivePhrasePtr phr) { this->decorateWithMembership(phr); });
  }
  m_p->m_qtModel->setVisibleIconURL(":/icons/display/selected.png");
  m_p->m_qtModel->setInvisibleIconURL(":/icons/display/unselected.png");
  if (m_p->m_phraseModel)
  {
    m_p->m_phraseModel->addSource(rsrcMgr, operMgr);
    m_p->m_modelObserverId =
      m_p->m_phraseModel->observe([this](smtk::view::DescriptivePhrasePtr phr,
        smtk::view::PhraseModelEvent evt, const std::vector<int>& src, const std::vector<int>& dst,
        const std::vector<int>& refs) { this->checkRemovedComponents(phr, evt, src, dst, refs); });
  }

  // Create a container for the item:
  m_widget = new QFrame(m_itemInfo.parentWidget());
  m_widget->installEventFilter(this);
  m_p->m_grid = new QGridLayout(m_widget);
  m_p->m_grid->setMargin(0);
  m_p->m_grid->setSpacing(0);
  m_p->m_grid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QSizePolicy sizeStretchyXPolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

  // Create a layout for the item's checkbox (if it is optional) and its label.
  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // Add the "enable" checkbox if the item is optional.
  int padding = 0;
  if (itm->isOptional())
  {
    m_p->m_optional = new QCheckBox(m_itemInfo.parentWidget());
    m_p->m_optional->setChecked(itm->isEnabled());
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
  m_p->m_label->setSizePolicy(sizeFixedPolicy);
  if (m_itemInfo.baseView())
  {
    m_p->m_label->setFixedWidth(m_itemInfo.baseView()->fixedLabelWidth() - padding);
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
  entryLayout->setMargin(0);
  entryLayout->setSpacing(6);
  entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // An entry consists of ...
  // ... a button to grab the selection
  QIcon copyFromSelection(":/icons/reference-item/copy-from-selection.png");
  m_p->m_copyFromSelection = new QPushButton(copyFromSelection, "");
  m_p->m_copyFromSelection->setSizePolicy(sizeFixedPolicy);
  m_p->m_copyFromSelection->setToolTip("Replace this item's members with the selection.");
  entryLayout->addWidget(m_p->m_copyFromSelection);
  QObject::connect(m_p->m_copyFromSelection, SIGNAL(clicked()), this, SLOT(copyFromSelection()));

  // ... a button to empty the item's members
  QIcon clearItem(":/icons/reference-item/clear.png");
  m_p->m_clear = new QPushButton(clearItem, "");
  m_p->m_clear->setSizePolicy(sizeFixedPolicy);
  m_p->m_clear->setToolTip("Clear this item's members.");
  entryLayout->addWidget(m_p->m_clear);
  QObject::connect(m_p->m_clear, SIGNAL(clicked()), this, SLOT(clearItem()));

  // ... a button to populate the selection with the item's members
  QIcon copyToSelection(":/icons/reference-item/copy-to-selection.png");
  m_p->m_copyToSelection = new QPushButton(copyToSelection, "");
  m_p->m_copyToSelection->setSizePolicy(sizeFixedPolicy);
  m_p->m_copyToSelection->setToolTip("Replace the selection with this item's members.");
  entryLayout->addWidget(m_p->m_copyToSelection);
  QObject::connect(m_p->m_copyToSelection, SIGNAL(clicked()), this, SLOT(copyToSelection()));

  // ... a synopsis (label).
  bool ok;
  QString synText = QString::fromStdString(this->synopsis(ok));
  m_p->m_synopsis = new QLabel(synText, m_widget);
  // m_p->m_synopsis->setSizePolicy(sizeFixedPolicy);
  m_p->m_synopsis->setSizePolicy(sizeStretchyXPolicy);
  m_p->m_synopsis->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  entryLayout->addWidget(m_p->m_synopsis);

  // ... a button to pop up an editor for the item contents.
  m_p->m_editBtn = new QPushButton("…", m_widget);
  m_p->m_editBtn->setAutoDefault(true);
  m_p->m_editBtn->setDefault(true);
  entryLayout->addWidget(m_p->m_editBtn);

  // Create a popup for editing the item's contents
  m_p->m_popup = new QDialog(m_p->m_editBtn);
  m_p->m_popup->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
  m_p->m_popupLayout = new QVBoxLayout(m_p->m_popup);
  m_p->m_popupList = new QListView(m_p->m_popup);
  m_p->m_popupList->setItemDelegate(m_p->m_qtDelegate);
  m_p->m_popupSynopsis = new QLabel(m_p->m_popup);
  m_p->m_popupSynopsis->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_p->m_popupDone = new QPushButton("Done", m_p->m_popup);
  auto hbl = new QHBoxLayout();
  hbl->addWidget(m_p->m_popupSynopsis);
  hbl->addWidget(m_p->m_popupDone);
  m_p->m_popupLayout->addWidget(m_p->m_popupList);
  m_p->m_popupLayout->addLayout(hbl);
  m_p->m_popup->installEventFilter(this);
  m_p->m_popupList->setModel(m_p->m_qtModel);

  QObject::connect(m_p->m_editBtn, SIGNAL(clicked()), m_p->m_popup, SLOT(exec()));
  QObject::connect(m_p->m_popupDone, SIGNAL(clicked()), this, SLOT(synchronizeAndHide()));
  QObject::connect(m_p->m_qtDelegate, SIGNAL(requestVisibilityChange(const QModelIndex&)),
    m_p->m_qtModel, SLOT(toggleVisibility(const QModelIndex&)));

  // ... a button to export the item contents to the selection:
  // ... a button to import the item contents from the selection:
  // ... a label (or button?) to indicate linkage of the selection with the item:

  m_p->m_grid->addLayout(labelLayout, 0, 0);
  m_p->m_grid->addLayout(entryLayout, 0, 1);

  // layout->addWidget(m_widget???, 0, 1);

  if (m_itemInfo.parentWidget() && m_itemInfo.parentWidget()->layout())
  {
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  if (itm->isOptional())
  {
    this->setOutputOptional(itm->isEnabled() ? 1 : 0);
  }
  this->synchronize(UpdateSource::GUI_FROM_ITEM);

  this->sneakilyHideButtons();
  this->updateSynopsisLabels();
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
  for (auto entry : m_p->m_members)
  {
    if (entry.second > 0)
    {
      ++numSel;
    }
  }
  ok = true;
  if (numRequired < 2 && maxAllowed == 1)
  {
    auto ment = (m_p->m_members.empty() ? smtk::resource::PersistentObjectPtr()
                                        : m_p->m_members.begin()->first);
    label << (numSel == 1 ? (ment ? ment->name() : "NULL!!")
                          : (numSel > 0 ? "too many" : "(none)"));
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
  if (!m_p || !m_p->m_synopsis || !m_p->m_popupSynopsis)
  {
    return;
  }

  bool ok = true;
  std::string syn = m_p->m_optional && !m_p->m_optional->isChecked() ? std::string("(disabled)")
                                                                     : this->synopsis(ok);
  QString qsyn = QString::fromStdString(syn);
  updateLabel(m_p->m_synopsis, qsyn, ok);
  updateLabel(m_p->m_popupSynopsis, qsyn, ok);
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
        // this->linkHover(true);
        this->cleverlyShowButtons();
        break;
      case QEvent::Leave:
        QTimer::singleShot(0, this, SLOT(linkHoverFalse()));
        // this->linkHover(false);
        this->sneakilyHideButtons();
        break;
      case QEvent::FocusIn:
        this->cleverlyShowButtons();
        QTimer::singleShot(0, this, SLOT(linkHoverTrue()));
        // this->linkHover(true);
        break;
      case QEvent::FocusOut:
        QTimer::singleShot(0, this, SLOT(linkHoverFalse()));
        this->sneakilyHideButtons();
        // this->linkHover(false);
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
      case QEvent::
        ShortcutOverride: // This is what keypresses look like to us (the parent of the QListView).
      {
        // std::cout << "  Popup key\n";
        auto keyEvent = static_cast<QKeyEvent*>(event);
        int kk = keyEvent->key();
        switch (kk)
        {
          case Qt::Key_Escape:
          case Qt::Key_Cancel:
            // std::cout << "    Hiding\n";
            this->synchronizeAndHide(true);
            return true;
            break;
          //case Qt::Key_Return:
          case Qt::Key_Enter:
          case Qt::Key_Space:
            // std::cout << "    Toggling\n";
            this->toggleCurrentItem();
            break;
          default:
            break;
        }
      }
      break;
      default:
        break;
    }
  }
  return false;
}

void qtReferenceItem::toggleCurrentItem()
{
  auto cphr = m_p->m_qtModel->getItem(m_p->m_popupList->currentIndex());
  if (cphr)
  {
    auto currentMembership = cphr->relatedVisibility();
    // Selecting a new item when only 1 is allowed should reset all other membership.
    if (!currentMembership && !m_p->m_members.empty())
    {
      auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
      if (item->numberOfRequiredValues() <= 1 && item->maxNumberOfValues() == 1)
      {
        m_p->m_members.clear();
        m_p->m_phraseModel->triggerDataChanged();
      }
    }
    cphr->setRelatedVisibility(!currentMembership);
    this->updateSynopsisLabels();
  }
}

int qtReferenceItem::decorateWithMembership(smtk::view::DescriptivePhrasePtr phr)
{
  smtk::view::VisibilityContent::decoratePhrase(
    phr, [this](smtk::view::VisibilityContent::Query qq, int val,
           smtk::view::ConstPhraseContentPtr data) {
      auto comp = data ? data->relatedComponent() : nullptr;
      auto rsrc = comp ? comp->resource() : (data ? data->relatedResource() : nullptr);
      auto pobj = comp ? std::dynamic_pointer_cast<smtk::resource::PersistentObject>(comp)
                       : std::dynamic_pointer_cast<smtk::resource::PersistentObject>(rsrc);

      switch (qq)
      {
        case smtk::view::VisibilityContent::DISPLAYABLE:
          return pobj ? 1 : 0;
        case smtk::view::VisibilityContent::EDITABLE:
          return pobj ? 1 : 0;
        case smtk::view::VisibilityContent::GET_VALUE:
          if (pobj)
          {
            auto valIt = m_p->m_members.find(pobj);
            if (valIt != m_p->m_members.end())
            {
              return valIt->second;
            }
            return 0; // visibility is assumed if there is no entry.
          }
          return 0; // visibility is false if the component is not a model entity or NULL.
        case smtk::view::VisibilityContent::SET_VALUE:
          if (pobj)
          {
            if (val && !m_p->m_members.empty())
            {
              auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
              if (item->numberOfRequiredValues() <= 1 &&
                (!item->isExtensible() || item->maxNumberOfValues() == 1))
              { // Clear all other members since only 1 is allowed and the user just chose it.
                m_p->m_members.clear();
                m_p->m_phraseModel->triggerDataChanged();
              }
            }
            if (val)
            {
              m_p->m_members[pobj] = val ? 1 : 0; // FIXME: Use a bit specified by the application.
            }
            else
            {
              m_p->m_members.erase(pobj);
            }
            this->updateSynopsisLabels();
            return 1;
          }
      }
      return 0;
    });
  return 0;
}

void qtReferenceItem::checkRemovedComponents(smtk::view::DescriptivePhrasePtr phr,
  smtk::view::PhraseModelEvent evt, const std::vector<int>& src, const std::vector<int>& dst,
  const std::vector<int>& refs)
{
  (void)phr;
  (void)dst;
  if (evt == smtk::view::PhraseModelEvent::ABOUT_TO_REMOVE)
  {
    bool didChange = false;
    auto itm = this->itemAs<smtk::attribute::ReferenceItem>();
    auto qidx = m_p->m_qtModel->indexFromPath(src);
    for (auto ref : refs)
    {
      auto ridx = m_p->m_qtModel->index(ref, 0, qidx);
      auto rphr = m_p->m_qtModel->getItem(ridx);
      auto comp = rphr ? rphr->relatedComponent() : nullptr;
      auto rsrc = rphr ? rphr->relatedResource() : nullptr;
      if (comp && m_p->m_members.find(comp) != m_p->m_members.end())
      {
        m_p->m_members.erase(comp);
        itm->removeValue(itm->find(comp));
        didChange = true;
      }
      else if (rsrc && m_p->m_members.find(rsrc) != m_p->m_members.end())
      {
        m_p->m_members.erase(rsrc);
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
  for (auto member : m_p->m_members)
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
      for (auto member : m_p->m_members)
      {
        if (member.second)
        {
          if (!item->setObjectValue(idx, member.first))
          {
            return false; // Huh!?!
          }
          ++idx;
        }
      }
      emit modified();
    }
    break;

    case UpdateSource::GUI_FROM_ITEM:
      m_p->m_members.clear();
      m_p->m_phraseModel->triggerDataChanged();
      for (auto vit = item->begin(); vit != item->end(); ++vit)
      {
        // Only allow non-null pointers into the set of selected items;
        // null pointers indicate that the item's entry is invalid and
        // the size of m_members is used to determine whether the
        // association's rules are met, so an extra entry can prevent
        // the association from being edited by the user.
        if (*vit)
        {
          m_p->m_members[*vit] = 1; // FIXME: Use a bit specified by the application.
        }
      }
      break;
  }
  return true;
}

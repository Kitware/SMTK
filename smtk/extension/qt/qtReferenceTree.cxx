//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtReferenceTree.h"
#include "smtk/extension/qt/qtReferenceTreeData.h"

#include "smtk/extension/qt/MembershipBadge.h"
#include "smtk/extension/qt/VisibilityBadge.h"
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
#include <QMouseEvent>
#include <QTimer>
#include <QWidgetAction>

using namespace smtk::extension;
using namespace smtk::attribute;
using MembershipBadge = smtk::extension::qt::MembershipBadge;

namespace
{
nlohmann::json defaultConfiguration = {
  { "Name", "RefItem" },
  { "Type", "smtk::view::ResourcePhraseModel" },
  { "Component",
    { { "Name", "Details" },
      { "Attributes", { { "TopLevel", true }, { "Title", "Resources" } } },
      { "Children",
        { { { "Name", "PhraseModel" },
            { "Attributes", { { "Type", "smtk::view::ResourcePhraseModel" } } },
            { "Children",
              { { { "Name", "SubphraseGenerator" },
                  { "Attributes", { { "Type", "smtk::view::SubphraseGenerator" } } } },
                { { "Name", "Badges" },
                  { "Children",
                    {
                      { { "Name", "Badge" },
                        { "Attributes",
                          { { "Default", false },
                            { "Type", "smtk::extension::qt::MembershipBadge" } } } },
                      { { "Name", "Badge" },
                        { "Attributes",
                          { { "Default", false },
                            { "Type",
                              "smtk::extension::paraview::appcomponents::VisibilityBadge" } } } },
                    } } } } } } } } } }
};
} // namespace

qtItem* qtReferenceTree::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  auto item = info.itemAs<smtk::attribute::ReferenceItem>();
  if (item == nullptr)
  {
    return nullptr;
  }
  auto* qi = new qtReferenceTree(info);
  // qtReferenceTree does not call createWidget in its
  // constructor (because that would cause problems for
  // subclasses since the method is virtual and could
  // be overridden). Call it now:
  qi->createWidget();
  return qi;
}

qtReferenceTreeData::qtReferenceTreeData()
  : m_membershipCriteria{ qt::MembershipCriteria::ComponentsWithGeometry }
  , m_selectedIconURL(":/icons/display/selected.png")
  , m_unselectedIconURL(":/icons/display/unselected.png")
{
}

qtReferenceTreeData::~qtReferenceTreeData() = default;

qtReferenceTree::qtReferenceTree(const qtAttributeItemInfo& info)
  : Superclass(info)
  , m_p(new qtReferenceTreeData)
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

qtReferenceTree::~qtReferenceTree()
{
  this->removeObservers();
  delete m_p;
  m_p = nullptr;
}

void qtReferenceTree::markForDeletion()
{
  this->removeObservers();
  qtItem::markForDeletion();
}

void qtReferenceTree::removeObservers()
{
  if (m_p->m_phraseModel && m_p->m_modelObserverId.assigned())
  {
    m_p->m_phraseModel->observers().erase(m_p->m_modelObserverId);
  }
  QObject::disconnect(this);
}

qt::MembershipCriteria qtReferenceTree::membershipCriteria() const
{
  return m_p->m_membershipCriteria;
}

std::string qtReferenceTree::membershipFilter() const
{
  return m_p->m_membershipFilter;
}

void qtReferenceTree::setLabelVisible(bool visible)
{
  m_p->m_label->setVisible(visible);
}

bool qtReferenceTree::setSelectionIconPaths(
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

std::pair<std::string, std::string> qtReferenceTree::selectionIconPaths() const
{
  return std::make_pair(m_p->m_selectedIconURL, m_p->m_unselectedIconURL);
}

std::shared_ptr<smtk::view::PhraseModel> qtReferenceTree::phraseModel() const
{
  return m_p->m_phraseModel;
}

void qtReferenceTree::updateItemData()
{
  this->updateUI();
  this->Superclass::updateItemData();
}

void qtReferenceTree::synchronizeMembers()
{
  this->synchronize(UpdateSource::ITEM_FROM_GUI);
}

void qtReferenceTree::setOutputOptional(int state)
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
}

void qtReferenceTree::linkHover(bool link)
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
    hover, "qtReferenceTreeHover", 0x02, smtk::view::SelectionAction::UNFILTERED_REPLACE, true);
  // TODO: traverse entries of m_itemInfo.item() and ensure their "hover" bit is set
  //       in the application selection.
}

void qtReferenceTree::linkHoverTrue()
{
  this->linkHover(true);
}

void qtReferenceTree::linkHoverFalse()
{
  this->linkHover(false);
}

void qtReferenceTree::copyFromSelection()
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
        if (m_p->m_highlightOnHover)
        {
          this->linkHover(true);
        }
        Q_EMIT modified();
      }
    }
  }
}

void qtReferenceTree::copyToSelection()
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
    seln->modifySelection(nextSeln, "qtReferenceTree", 1); // FIXME: Use an app-specified bit
  }
}

void qtReferenceTree::clearItem()
{
  m_itemInfo.item()->reset();
  if (this->synchronize(UpdateSource::GUI_FROM_ITEM))
  {
    if (m_p->m_highlightOnHover)
    {
      this->linkHover(true);
    }
    Q_EMIT modified();
  }
}

smtk::view::PhraseModelPtr qtReferenceTree::createPhraseModel() const
{
  // Construct the PhraseModel with a factory from our config which
  // must include a MembershipBadge. Check that it does. We may
  // modify the membership badge's configuration if only a single
  // component may be selected.
  auto refItem = std::dynamic_pointer_cast<smtk::attribute::ReferenceItem>(m_itemInfo.item());
  bool haveConfig = true;
  auto comp = m_itemInfo.component();
  int pmi = comp.findChild("PhraseModel");
  int bdi = -1;
  int mbi = -1;
  if (pmi >= 0)
  {
    auto& pmc = comp.child(pmi);
    bdi = pmc.findChild("Badges");
    bool haveMembershipBadge = false;
    if (bdi >= 0)
    {
      auto& bdc = pmc.child(bdi);
      for (mbi = 0; mbi < static_cast<int>(bdc.numberOfChildren()); ++mbi)
      {
        auto& badge = bdc.child(mbi);
        std::string badgeType;
        if (
          badge.name() == "Badge" && badge.attribute("Type", badgeType) &&
          badgeType == "smtk::extension::qt::MembershipBadge")
        {
          haveMembershipBadge = true;
          if (
            refItem && refItem->numberOfRequiredValues() < 2 &&
            ((!refItem->isExtensible()) ||
             (refItem->isExtensible() && refItem->maxNumberOfValues() == 1)))
          {
            // Configure the badge selection mechanism to match the reference item:
            // If the item only allows single selection, configure the membership badge for it.
            badge.setAttribute("SingleSelect", "true");
          }
          break;
        }
      }
      if (!haveMembershipBadge)
      {
        haveConfig = false;
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Unconfigured qtReferenceTree (no membership badge)!");
      }
    }
    else
    {
      haveConfig = false;
      smtkErrorMacro(smtk::io::Logger::instance(), "Unconfigured qtReferenceTree (no badges)!");
      // pmc.addChild("Badge") ... add Type="smtk::extension::qt::MembershipBadge", etc.
    }
  }
  else
  {
    haveConfig = false;
    smtkErrorMacro(smtk::io::Logger::instance(), "Unconfigured qtReferenceTree (no phrase model)!");
  }
  smtk::view::ConfigurationPtr phraseModelConfig;
  json jj;
  if (!haveConfig)
  {
    // If we don't have a proper configuration, use a default one:
    jj = defaultConfiguration;
    // If the item only allows single selection, configure the membership badge for it.
    if (
      refItem && refItem->numberOfRequiredValues() < 2 &&
      ((!refItem->isExtensible()) ||
       (refItem->isExtensible() && refItem->maxNumberOfValues() == 1)))
    {
      jj["Component"]["Children"][0]["Children"][1]["Children"][0]["Attributes"]["SingleSelect"] =
        true;
    }
    // Hardwire membership filter for now
    jj["Component"]["Children"][0]["Children"][1]["Children"][0]["Attributes"]["Filter"] =
      this->membershipFilter();
    jj["Component"]["Children"][0]["Children"][1]["Children"][0]["Attributes"]
      ["MembershipCriteria"] = membershipCriteriaName(this->membershipCriteria());
    phraseModelConfig = jj;
  }
  else
  {
    // We have a proper item configuration, but we need to create a view configuration
    // for the phrase model (it behaves like a view, not an item; we just embed it in
    // the item). So, construct one via JSON:
    jj = { { "Name", "RefItem" }, { "Type", "smtk::view::ResourcePhraseModel" } };
    phraseModelConfig = jj;
    phraseModelConfig->details() = comp;
  }
  auto phraseModel =
    m_itemInfo.uiManager()->viewManager()->phraseModelFactory().createFromConfiguration(
      phraseModelConfig.get());
  auto* membershipBadge =
    phraseModel->badges().findBadgeOfType<smtk::extension::qt::MembershipBadge>();
  QObject::connect(
    membershipBadge,
    &MembershipBadge::membershipChange,
    this,
    &qtReferenceTree::synchronizeMembers);
  // TODO: None of the following belongs here... we should pass the common::Managers
  //       to the factory and it should contain the ReferenceItem so the phrase model
  //       can configure itself.
  phraseModel->addSource(m_itemInfo.uiManager()->managers());
  auto def = std::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(
    m_itemInfo.item()->definition());
  if (auto ripm = std::dynamic_pointer_cast<smtk::view::ReferenceItemPhraseModel>(phraseModel))
  {
    ripm->setReferenceItem(refItem);
  }
  return phraseModel;
}

void qtReferenceTree::createWidget()
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

void qtReferenceTree::clearWidgets()
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

void qtReferenceTree::updateUI()
{
  smtk::attribute::ItemPtr itm = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(itm))
  {
    return;
  }

  // TODO: this needs to connect to the right managers

  auto phraseModel = this->createPhraseModel();
  m_p->m_phraseModel = phraseModel;
  m_p->m_qtModel = new qtDescriptivePhraseModel;
  m_p->m_qtModel->setPhraseModel(m_p->m_phraseModel);
  m_p->m_qtDelegate = new smtk::extension::qtDescriptivePhraseDelegate;
  int textVerticalPad = 6;
  int titleFontWeight = 1;
  bool drawSubtitle = false;
  bool visibilityMode = true;
  m_itemInfo.component().attributeAsInt("TextVerticalPad", textVerticalPad);
  m_itemInfo.component().attributeAsInt("TitleFontWeight", titleFontWeight);
  m_itemInfo.component().attributeAsBool("DrawSubtitle", drawSubtitle);
  m_itemInfo.component().attributeAsBool("VisibilityMode", visibilityMode);
  m_itemInfo.component().attributeAsBool("HighlightOnHover", m_p->m_highlightOnHover);
  m_p->m_qtDelegate->setTextVerticalPad(textVerticalPad);
  m_p->m_qtDelegate->setTitleFontWeight(titleFontWeight);
  m_p->m_qtDelegate->setDrawSubtitle(drawSubtitle);
  m_p->m_qtDelegate->setVisibilityMode(visibilityMode);

  if (m_p->m_phraseModel)
  {
    m_p->m_phraseModel->addSource(m_itemInfo.uiManager()->managers());
    QPointer<qtReferenceTree> guardedObject(this);
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
      "qtReferenceTree: Check for removed components.");
    // we need to know when membership is changed, to update our labels
    MembershipBadge* badge =
      m_p->m_phraseModel->badges().findBadgeOfType<smtk::extension::qt::MembershipBadge>();
    QObject::connect(
      badge, &MembershipBadge::membershipChange, this, &qtReferenceTree::membershipChanged);
  }

  // Create a container for the item:
  if (m_widget)
  {
    delete m_widget;
  }

  m_widget = new QFrame(m_itemInfo.parentWidget());
  m_widget->setObjectName("ReferenceTreeFrame");
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
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
  m_p->m_label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    m_p->m_label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  m_p->m_label->setWordWrap(true);
  m_p->m_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // Add in BriefDescription as tooltip if available:
  std::string strBriefDescription = itemDef->briefDescription();
  if (strBriefDescription.empty())
  {
    strBriefDescription = "Space key changes membership<br>Return key changes visibility";
  }
  m_p->m_label->setToolTip(strBriefDescription.c_str());

  if (itemDef->advanceLevel() && m_itemInfo.uiManager())
  {
    m_p->m_label->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  labelLayout->addWidget(m_p->m_label);

  // Now add widgetry for the "entry"
  // Create a layout for the item's entry editor.
  QVBoxLayout* entryLayout = new QVBoxLayout();
  entryLayout->setMargin(0);
  entryLayout->setSpacing(6);
  entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  QHBoxLayout* headerLayout = new QHBoxLayout();
  headerLayout->setMargin(0);
  headerLayout->setSpacing(6);
  headerLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // An entry consists of ...
  // ... a button to grab the selection
  QIcon copyFromSelection(":/icons/reference-item/copy-from-selection.png");
  m_p->m_copyFromSelection = new QPushButton(copyFromSelection, "");
  m_p->m_copyFromSelection->setSizePolicy(sizeFixedPolicy);
  m_p->m_copyFromSelection->setToolTip("Replace this item's members with the selection.");
  m_p->m_copyFromSelection->setObjectName("CopyFromSelection");
  headerLayout->addWidget(m_p->m_copyFromSelection);
  QObject::connect(m_p->m_copyFromSelection, SIGNAL(clicked()), this, SLOT(copyFromSelection()));

  // ... a button to empty the item's members
  QIcon clearItem(":/icons/reference-item/clear.png");
  m_p->m_clear = new QPushButton(clearItem, "");
  m_p->m_clear->setSizePolicy(sizeFixedPolicy);
  m_p->m_clear->setToolTip("Clear this item's members.");
  m_p->m_clear->setObjectName("ClearMembership");
  headerLayout->addWidget(m_p->m_clear);
  QObject::connect(m_p->m_clear, SIGNAL(clicked()), this, SLOT(clearItem()));

  // ... a button to populate the selection with the item's members
  QIcon copyToSelection(":/icons/reference-item/copy-to-selection.png");
  m_p->m_copyToSelection = new QPushButton(copyToSelection, "");
  m_p->m_copyToSelection->setSizePolicy(sizeFixedPolicy);
  m_p->m_copyToSelection->setToolTip("Replace the selection with this item's members.");
  m_p->m_copyToSelection->setObjectName("CopyToSelection");
  headerLayout->addWidget(m_p->m_copyToSelection);
  QObject::connect(m_p->m_copyToSelection, SIGNAL(clicked()), this, SLOT(copyToSelection()));

  // Create a descriptive-phrase tree-view for editing the item's contents
  auto refItem = m_itemInfo.itemAs<smtk::attribute::ReferenceItem>();
  bool multiselect = refItem &&
    (refItem->numberOfRequiredValues() > 1 ||
     (refItem->isExtensible() && refItem->maxNumberOfValues() != 1));
  m_p->m_view = new QTreeView(); // was m_p->m_popup
  m_p->m_view->setItemDelegate(m_p->m_qtDelegate);
  m_p->m_view->installEventFilter(this); // was m_p->m_popup
  m_p->m_view->viewport()->installEventFilter(this);
  m_p->m_view->setModel(m_p->m_qtModel);
  m_p->m_view->setSelectionMode(
    multiselect ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
  m_p->m_view->setSelectionBehavior(QAbstractItemView::SelectRows);

  entryLayout->setObjectName("ReferenceTreeLayout");
  headerLayout->setObjectName("ReferenceTreeHeader");
  entryLayout->addItem(headerLayout);
  entryLayout->addWidget(m_p->m_view);

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
}

bool qtReferenceTree::eventFilter(QObject* src, QEvent* event)
{
  // We serve as an event filter on 2 widgets:
  // 1. the inherited frame holding the item (m_widget),
  //    which we monitor for the enter/exit events that enable hovering
  // 2. the tree view (m_p->m_view)
  //    which we monitor for keyboard navigation events
  if (src == m_widget)
  {
    switch (event->type())
    {
      case QEvent::Enter:
        if (m_p->m_highlightOnHover)
        {
          QTimer::singleShot(0, this, SLOT(linkHoverTrue()));
        }
        break;
      case QEvent::Leave:
        if (m_p->m_highlightOnHover)
        {
          QTimer::singleShot(0, this, SLOT(linkHoverFalse()));
        }
        break;
      case QEvent::FocusIn:
        if (m_p->m_highlightOnHover)
        {
          QTimer::singleShot(0, this, SLOT(linkHoverTrue()));
        }
        break;
      case QEvent::FocusOut:
        if (m_p->m_highlightOnHover)
        {
          QTimer::singleShot(0, this, SLOT(linkHoverFalse()));
        }
        break;
      default:
        break;
    }
  }
  else if (src == m_p->m_view)
  {
    // std::cout << "Popup event: " << event->type() << "\n";
    switch (event->type())
    {
      case QEvent::KeyPress:
        // case QEvent::ShortcutOverride: // What keypresses look like to the parent of the QListView.
        {
          // std::cout << "  Popup key\n";
          auto* keyEvent = static_cast<QKeyEvent*>(event);
          int kk = keyEvent->key();
          switch (kk)
          {
            case Qt::Key_Escape:
            case Qt::Key_Cancel:
              // std::cout << "    Hiding\n";
              // this->synchronizeAndHide(true);
              return true;
              break;
            case Qt::Key_Return:
            case Qt::Key_Enter:
              // this->synchronizeAndHide(false);
              this->toggleCurrentItem(false);
              return true;
              break;
            case Qt::Key_Space:
              // std::cout << "    Toggling etype " << keyEvent->type() << "\n";
              this->toggleCurrentItem(true);
              // this->synchronize(UpdateSource::ITEM_FROM_GUI);
              return true;
              break;
            default:
              break;
          }
        }
        break;
      case QEvent::Hide:
      {
        // if (m_p->m_popup->isVisible())
        {
          // The user has clicked outside the popup.
          // Decide whether to update the item state or abandon.
          // this->synchronizeAndHide(false);
          return true;
        }
      }
      break;
      default:
        break;
    }
  }
  else if (
    (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) &&
    m_p->m_view && m_p->m_view->isVisible() && src == m_p->m_view->viewport())
  {
    if (qtDescriptivePhraseDelegate::processBadgeClick(
          static_cast<QMouseEvent*>(event), m_p->m_view))
    {
      return true;
    }
  }
  return false; // QObject::eventFilter(src, event);
}

void qtReferenceTree::toggleCurrentItem(bool membership)
{
  auto cphr = m_p->m_view->currentIndex()
                .data(smtk::extension::qtDescriptivePhraseModel::PhrasePtrRole)
                .value<smtk::view::DescriptivePhrasePtr>();
  if (cphr)
  {
    auto persistentObj = cphr->relatedObject();
    smtk::view::Badge* badge = membership
      ? dynamic_cast<smtk::view::Badge*>(
          m_p->m_phraseModel->badges().findBadgeOfType<smtk::extension::qt::MembershipBadge>())
      : dynamic_cast<smtk::view::Badge*>(
          m_p->m_phraseModel->badges().findBadgeOfType<smtk::extension::qt::VisibilityBadge>());
    auto selected = m_p->m_view->selectionModel()->selection();
    badge->action(cphr.get(), smtk::extension::qtBadgeActionToggle(selected));
  }
}

void qtReferenceTree::membershipChanged(int)
{
  m_p->m_phraseModel->triggerDataChanged(); // Any change in membership should cause a redraw.
  if (m_p->m_highlightOnHover)
  {
    this->linkHoverTrue();
  }
}

void qtReferenceTree::checkRemovedComponents(
  smtk::view::DescriptivePhrasePtr phr,
  smtk::view::PhraseModelEvent evt,
  const std::vector<int>& src,
  const std::vector<int>& dst,
  const std::vector<int>& refs)
{
  (void)phr;
  (void)dst;
  bool didChange = false;
  if (evt == smtk::view::PhraseModelEvent::ABOUT_TO_REMOVE)
  {
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
  }
  (void)didChange; // Return this?
}

bool qtReferenceTree::synchronize(UpdateSource src)
{
  bool didChange = false;
  auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
  if (!item)
  {
    return didChange;
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
      auto itemMembers = static_cast<std::size_t>(item->numberOfValues());
      // Everything else in this case statement should really be
      // a single, atomic operation executed on the attribute/item:
      bool resizedProperly = item->setNumberOfValues(uiMembers);
      auto newItemMembers = static_cast<std::size_t>(item->numberOfValues());
      if (!resizedProperly)
      {
        if (newItemMembers < uiMembers)
        {
          // We are going to lose entries because the item cannot hold them all.
        }
      }
      didChange |= (newItemMembers != itemMembers);
      int idx = 0;
      for (const auto& member : this->members())
      {
        if (member.second && idx < static_cast<int>(newItemMembers))
        {
          auto entry = member.first.lock();
          // If the new entry is not identical to the existing value, set it.
          bool didSet = false;
          if ((item->isSet(idx) && item->value(idx) != entry) || !item->isSet(idx))
          {
            didSet = item->setValue(idx, entry);
            if (!didSet)
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(), "qtReferenceTree: Failed to add " << entry->name());
              return didChange; // Huh!?!
            }
            else
            {
              didChange = true;
            }
          }
          ++idx;
        }
      }
      // Now unset any entries that we cannot resize away.
      for (; idx < static_cast<int>(newItemMembers); ++idx)
      {
        if (item->isSet())
        {
          item->unset(idx);
          didChange = true;
        }
      }
      if (didChange)
      {
        Q_EMIT modified();
      }
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

smtk::extension::qt::MembershipBadge::MemberMap& qtReferenceTree::members() const
{
  // We expect there to be only one MembershipBadge on our phraseModel
  return (m_p->m_phraseModel->badges().findBadgeOfType<smtk::extension::qt::MembershipBadge>())
    ->getMemberMap();
}

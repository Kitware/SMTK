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

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include "smtk/environment/Environment.h"

#include <QEvent>
#include <QKeyEvent>

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

qtReferenceItemData::qtReferenceItemData()
  : m_optional(nullptr)
{
}

qtReferenceItemData::~qtReferenceItemData()
{
}

qtReferenceItem::qtReferenceItem(smtk::attribute::ItemPtr item, QWidget* parent, qtBaseView* bview)
  : Superclass(item, parent, bview)
  , m_p(new qtReferenceItemData)
{
}

qtReferenceItem::~qtReferenceItem()
{
  delete m_p;
  m_p = nullptr;
}

void qtReferenceItem::selectionLinkToggled(bool linked)
{
  (void)linked;
}

void qtReferenceItem::setOutputOptional(int state)
{
  auto itm = this->getObject();
  if (itm)
  {
    itm->setIsEnabled(state ? true : false);
  }
  m_p->m_editBtn->setEnabled(state ? true : false);
  this->updateSynopsisLabels();
}

void qtReferenceItem::linkHover(bool link)
{
  (void)link;
  // TODO: traverse entries of this->getObject() and ensure their "hover" bit is set
  //       in the application selection.
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

void qtReferenceItem::createWidget()
{
  this->clearWidgets();
  this->updateUI();
}

void qtReferenceItem::clearWidgets()
{
  /*
  if (m_p->m_grid)
  {
    delete m_p->m_grid;
  }
  */
}

void qtReferenceItem::updateUI()
{
  smtk::attribute::ItemPtr itm = this->getObject();
  if (!itm || !this->passAdvancedCheck() ||
    (this->baseView() && !this->baseView()->uiManager()->passItemCategoryCheck(itm->definition())))
  {
    return;
  }

  auto rsrcMgr = smtk::environment::ResourceManager::instance();
  auto operMgr = smtk::environment::OperationManager::instance();

  auto phraseModel = this->createPhraseModel();
  m_p->m_phraseModel = phraseModel;
  m_p->m_qtModel = new qtDescriptivePhraseModel;
  m_p->m_qtModel->setPhraseModel(m_p->m_phraseModel);
  m_p->m_qtDelegate = new smtk::extension::qtDescriptivePhraseDelegate;
  m_p->m_qtDelegate->setTextVerticalPad(6);
  m_p->m_qtDelegate->setTitleFontWeight(1);
  m_p->m_qtDelegate->setDrawSubtitle(false);
  m_p->m_qtDelegate->setVisibilityMode(true);
  m_p->m_phraseModel->setDecorator(
    [this](smtk::view::DescriptivePhrasePtr phr) { this->decorateWithMembership(phr); });
  m_p->m_qtModel->setVisibleIconURL(":/icons/display/selected.png");
  m_p->m_qtModel->setInvisibleIconURL(":/icons/display/unselected.png");
  m_p->m_phraseModel->addSource(rsrcMgr, operMgr);

  // Create a container for the item:
  this->Widget = new QFrame(this->parentWidget());
  this->Widget->installEventFilter(this);
  m_p->m_grid = new QGridLayout(this->Widget);
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
    m_p->m_optional = new QCheckBox(this->parentWidget());
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
  m_p->m_label = new QLabel(labelText, this->Widget);
  m_p->m_label->setSizePolicy(sizeFixedPolicy);
  if (this->baseView())
  {
    m_p->m_label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
  }
  m_p->m_label->setWordWrap(true);
  m_p->m_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // Add in BriefDescription as tooltip if available:
  const std::string strBriefDescription = itemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    m_p->m_label->setToolTip(strBriefDescription.c_str());
  }

  if (itemDef->advanceLevel() && this->baseView())
  {
    m_p->m_label->setFont(this->baseView()->uiManager()->advancedFont());
  }
  labelLayout->addWidget(m_p->m_label);

  // Now add widgetry for the "entry"
  // Create a layout for the item's entry editor.
  QHBoxLayout* entryLayout = new QHBoxLayout();
  entryLayout->setMargin(0);
  entryLayout->setSpacing(6);
  entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // An entry consists of ...
  // ... a synopsis (label).
  bool ok;
  QString synText = QString::fromStdString(this->synopsis(ok));
  m_p->m_synopsis = new QLabel(synText, this->Widget);
  // m_p->m_synopsis->setSizePolicy(sizeFixedPolicy);
  m_p->m_synopsis->setSizePolicy(sizeStretchyXPolicy);
  m_p->m_synopsis->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  entryLayout->addWidget(m_p->m_synopsis);

  // ... a button to pop up an editor for the item contents.
  m_p->m_editBtn = new QPushButton("…", this->Widget);
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

  // layout->addWidget(this->Widget???, 0, 1);

  if (this->parentWidget() && this->parentWidget()->layout())
  {
    this->parentWidget()->layout()->addWidget(this->Widget);
  }
  if (itm->isOptional())
  {
    this->setOutputOptional(itm->isEnabled() ? 1 : 0);
  }
  this->synchronize(UpdateSource::GUI_FROM_ITEM);

  this->updateSynopsisLabels();
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
  // 1. the inherited frame holding the item (this->Widget),
  //    which we monitor for the enter/exit events that enable hovering
  // 2. the popup dialog (m_p->m_popup)
  //    which we monitor for keyboard navigation events
  if (src == this->Widget)
  {
    switch (event->type())
    {
      case QEvent::Enter:
        this->linkHover(true);
        break;
      case QEvent::Leave:
        this->linkHover(false);
        break;
      case QEvent::FocusIn:
        this->linkHover(true);
        break;
      case QEvent::FocusOut:
        this->linkHover(false);
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

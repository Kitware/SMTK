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

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

using namespace smtk::extension;
using namespace smtk::attribute;

qtReferenceItemData::qtReferenceItemData()
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

  this->Widget = new QFrame(this->parentWidget());
  m_p->m_grid = new QGridLayout(this->Widget);
  m_p->m_grid->setMargin(0);
  m_p->m_grid->setSpacing(0);
  m_p->m_grid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

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
  auto itemDef = itm->definition();

  QString labelText = !itm->label().empty() ? itm->label().c_str() : itm->name().c_str();
  m_p->m_label = new QLabel(labelText, this->Widget);
  m_p->m_label->setSizePolicy(sizeFixedPolicy);
  if (this->baseView())
  {
    m_p->m_label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
  }
  m_p->m_label->setWordWrap(true);
  m_p->m_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // add in BriefDescription as tooltip if available
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

  m_p->m_grid->addLayout(labelLayout, 0, 0);
  // layout->addWidget(this->Widget???, 0, 1);
  if (this->parentWidget() && this->parentWidget()->layout())
  {
    this->parentWidget()->layout()->addWidget(this->Widget);
  }
  if (itm->isOptional())
  {
    this->setOutputOptional(itm->isEnabled() ? 1 : 0);
  }
}

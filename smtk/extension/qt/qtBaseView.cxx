//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/common/View.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QScrollArea>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

using namespace smtk::extension;

class qtBaseViewInternals
{
public:
  qtBaseViewInternals() {}
  ~qtBaseViewInternals() { this->clearWidgets(); }
  void deleteWidget(QWidget* w)
  {
    if (w)
    {
      delete w;
    }
  }
  void clearWidgets()
  {
    this->deleteWidget(this->AdvLevelCombo);
    this->deleteWidget(this->ShowCategoryCombo);
    this->deleteWidget(this->FilterByCheck);
    this->deleteWidget(this->AdvLevelEditButton);
    this->deleteWidget(this->AdvLevelLabel);
    if (this->TopLevelLayout)
    {
      delete this->TopLevelLayout;
    }
  }

  QPointer<QComboBox> AdvLevelCombo;
  QPointer<QCheckBox> FilterByCheck;
  QPointer<QComboBox> ShowCategoryCombo;
  QPointer<QLabel> AdvLevelLabel;
  QPointer<QToolButton> AdvLevelEditButton;
  QPointer<QHBoxLayout> TopLevelLayout;
};

qtBaseView::qtBaseView(const ViewInfo& info)
{
  this->m_viewInfo = info;
  this->Internals = new qtBaseViewInternals;
  this->m_fixedLabelWidth = this->m_viewInfo.m_UIManager->maxValueLabelLength();
  this->Widget = NULL;
  this->m_advOverlayVisible = false;
  this->m_ScrollArea = NULL;
  this->m_isTopLevel = false;
  this->m_topLevelInitialized = false;
  if (this->m_viewInfo.m_view)
  {
    this->m_isTopLevel = this->m_viewInfo.m_view->details().attributeAsBool("TopLevel");
  }
}

qtBaseView::~qtBaseView()
{
  if (this->Internals)
  {
    delete this->Internals;
  }
  if (this->m_ScrollArea)
  {
    delete this->m_ScrollArea;
  }
}

void qtBaseView::getDefinitions(
  smtk::attribute::DefinitionPtr attDef, QList<smtk::attribute::DefinitionPtr>& defs)
{
  std::vector<smtk::attribute::DefinitionPtr> newdefs;
  attribute::System* attSystem = attDef->system();
  attSystem->findAllDerivedDefinitions(attDef, true, newdefs);
  if (!attDef->isAbstract() && !defs.contains(attDef))
  {
    defs.push_back(attDef);
  }
  std::vector<smtk::attribute::DefinitionPtr>::iterator itDef;
  for (itDef = newdefs.begin(); itDef != newdefs.end(); ++itDef)
  {
    if (!(*itDef)->isAbstract() && !defs.contains(*itDef))
    {
      defs.push_back(*itDef);
    }
  }
}

bool qtBaseView::displayItem(smtk::attribute::ItemPtr item)
{
  if (!item)
  {
    return false;
  }
  return this->uiManager()->passAdvancedCheck(item->advanceLevel()) &&
    this->uiManager()->passItemCategoryCheck(item->definition());
}

void qtBaseView::valueChanged(smtk::attribute::ItemPtr item)
{
  emit this->modified(item);
  this->uiManager()->onViewUIModified(this, item);
}

bool qtBaseView::setFixedLabelWidth(int w)
{
  w = std::min(w, this->uiManager()->maxValueLabelLength());
  w = std::max(w, this->uiManager()->minValueLabelLength());
  this->m_fixedLabelWidth = w;
  return false;
}

void qtBaseView::buildUI()
{
  if (this->m_isTopLevel && (!this->m_topLevelInitialized))
  {
    // Process the aspects associate with top level views
    this->makeTopLevel();
  }

  // Build the View's Main Widget
  this->createWidget();

  if (!(this->Widget && this->parentWidget()))
  {
    // Should say some kind of error
    return;
  }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());

  if (!parentlayout)
  {
    // Should say some kind of error or maybe create one?
    return;
  }

  if (!this->isTopLevel())
  {
    parentlayout->setAlignment(Qt::AlignTop);
    parentlayout->addWidget(this->Widget);
    return;
  }

  if (!this->m_ScrollArea)
  {
    // This should be an error!
    return;
  }

  this->m_ScrollArea->setWidget(this->Widget);
}

void qtBaseView::makeTopLevel()
{

  smtk::common::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  this->m_topLevelInitialized = true;
  int pos;
  std::vector<double> vals;
  pos = view->details().findChild("DefaultColor");
  QColor color;
  if (pos != -1)
  {
    view->details().child(pos).contentsAsVector(vals);
    if (vals.size() == 3)
    {
      color.setRgbF(vals[0], vals[1], vals[2]);
      this->uiManager()->setDefaultValueColor(color);
    }
  }
  pos = view->details().findChild("InvalidColor");
  if (pos != -1)
  {
    view->details().child(pos).contentsAsVector(vals);
    if (vals.size() == 3)
    {
      color.setRgbF(vals[0], vals[1], vals[2]);
      this->uiManager()->setInvalidValueColor(color);
    }
  }

  pos = view->details().findChild("AdvancedFontEffects");
  if (pos != -1)
  {
    bool val;
    if (!view->details().child(pos).attributeAsBool("Bold", val))
    {
      this->uiManager()->setAdvanceFontStyleBold(val);
    }
    if (!view->details().child(pos).attributeAsBool("Italic", val))
    {
      this->uiManager()->setAdvanceFontStyleItalic(val);
    }
  }

  pos = view->details().findChild("MaxValueLabelLength");
  if (pos != -1)
  {
    int l;
    if (view->details().child(pos).contentsAsInt(l))
    {
      this->uiManager()->setMaxValueLabelLength(l);
    }
  }

  pos = view->details().findChild("MinValueLabelLength");
  if (pos != -1)
  {
    int l;
    if (view->details().child(pos).contentsAsInt(l))
    {
      this->uiManager()->setMinValueLabelLength(l);
    }
  }

  this->Internals->clearWidgets();
  const attribute::System* attSys = this->uiManager()->attSystem();

  bool flag;
  // Do we need to provide advance level filtering? - this is on by default
  if ((!view->details().attributeAsBool("FilterByAdvanceLevel", flag)) || flag)
  {
    this->Internals->AdvLevelCombo = new QComboBox(this->parentWidget());
    this->uiManager()->initAdvanceLevels(this->Internals->AdvLevelCombo);

    this->Internals->AdvLevelLabel = new QLabel("Show Level:");
    this->Internals->AdvLevelLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  }

  // Do we need to show the advance level for each item?
  if (view->details().attributeAsBool("DisplayItemAccessLevel"))
  {
    QToolButton* editButton = new QToolButton(this->parentWidget());
    editButton->setCheckable(true);
    QString resourceName(":/icons/attribute/lock.png");
    editButton->setFixedSize(QSize(20, 20));
    editButton->setIcon(QIcon(resourceName));
    editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    editButton->setToolTip("Edit access level");
    connect(editButton, SIGNAL(toggled(bool)), this, SLOT(showAdvanceLevelOverlay(bool)));
    this->Internals->AdvLevelEditButton = editButton;
  }

  // Do we need to provide category filtering - this is on by default
  if ((!view->details().attributeAsBool("FilterByCategory", flag)) || flag)
  {
    this->Internals->FilterByCheck = new QCheckBox(this->parentWidget());
    this->Internals->FilterByCheck->setText("Show by Category: ");
    this->Internals->FilterByCheck->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->Internals->ShowCategoryCombo = new QComboBox(this->parentWidget());
    std::set<std::string>::const_iterator it;
    const std::set<std::string>& cats = attSys->categories();
    for (it = cats.begin(); it != cats.end(); it++)
    {
      this->Internals->ShowCategoryCombo->addItem(it->c_str());
    }
    this->Internals->ShowCategoryCombo->setEnabled(false);
  }

  this->Internals->TopLevelLayout = new QHBoxLayout();
  if (this->Internals->AdvLevelEditButton)
  {
    this->Internals->TopLevelLayout->addWidget(this->Internals->AdvLevelEditButton);
  }
  if (this->Internals->AdvLevelLabel)
  {
    this->Internals->TopLevelLayout->addWidget(this->Internals->AdvLevelLabel);
    this->Internals->TopLevelLayout->addWidget(this->Internals->AdvLevelCombo);
    QObject::connect(this->Internals->AdvLevelCombo, SIGNAL(currentIndexChanged(int)), this,
      SLOT(onAdvanceLevelChanged(int)));
  }
  if (this->Internals->FilterByCheck)
  {
    this->Internals->TopLevelLayout->addWidget(this->Internals->FilterByCheck);
    this->Internals->TopLevelLayout->addWidget(this->Internals->ShowCategoryCombo);

    QObject::connect(
      this->Internals->FilterByCheck, SIGNAL(stateChanged(int)), this, SLOT(enableShowBy(int)));
    QObject::connect(this->Internals->ShowCategoryCombo, SIGNAL(currentIndexChanged(int)), this,
      SLOT(onShowCategory()));
  }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addLayout(this->Internals->TopLevelLayout);

  this->m_ScrollArea = new QScrollArea(this->parentWidget());
  this->m_ScrollArea->setWidgetResizable(true);
  this->m_ScrollArea->setFrameShape(QFrame::NoFrame);
  this->m_ScrollArea->setObjectName("topLevelScrollArea");
  parentlayout->addWidget(this->m_ScrollArea);
}

void qtBaseView::showAdvanceLevel(int level)
{
  // If this is not a toplevel widget don't do anything
  if (!this->m_isTopLevel)
  {
    return;
  }

  smtk::common::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  // Are we filtering on advance level info?
  if (this->Internals->AdvLevelCombo)
  {
    this->Internals->AdvLevelCombo->blockSignals(true);
    for (int i = 0; i < this->Internals->AdvLevelCombo->count(); i++)
    {
      int l = this->Internals->AdvLevelCombo->itemData(i).toInt();
      if (level == l)
      {
        this->Internals->AdvLevelCombo->setCurrentIndex(i);
        break;
      }
    }
    this->Internals->AdvLevelCombo->blockSignals(false);
  }

  this->uiManager()->setAdvanceLevel(level);
  if (this->advanceLevelVisible())
  {
    this->showAdvanceLevelOverlay(true);
  }
  if (this->Widget)
  {
    delete this->Widget;
    this->Widget = NULL;
  }
  this->buildUI();
}

void qtBaseView::enableShowBy(int enable)
{
  this->Internals->ShowCategoryCombo->setEnabled(enable ? true : false);
  this->onShowCategory();
}

std::string qtBaseView::currentCategory()
{
  return this->categoryEnabled() ? this->Internals->ShowCategoryCombo->currentText().toStdString()
                                 : "";
}

bool qtBaseView::categoryEnabled()
{
  return this->Internals->ShowCategoryCombo && this->Internals->ShowCategoryCombo->isEnabled();
}

void qtBaseView::onAdvanceLevelChanged(int levelIdx)
{
  // If this is not a toplevel widget don't do anything
  if (!this->m_isTopLevel)
  {
    return;
  }

  int level = this->Internals->AdvLevelCombo->itemData(levelIdx).toInt();
  this->showAdvanceLevel(level);
}

int qtBaseView::advanceLevel()
{
  return this->Internals->AdvLevelCombo->currentIndex();
}

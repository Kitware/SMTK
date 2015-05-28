//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtRootView.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtGroupView.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QScrollArea>
#include <QComboBox>
#include <QToolButton>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtRootViewInternals
{
public:

  QPointer<QComboBox> AdvLevelCombo;
  QPointer<qtGroupView> TabGroup;
  QPointer<QCheckBox> FilterByCheck;
  QPointer<QComboBox> ShowCategoryCombo;
  QPointer<QLabel> AdvLevelLabel;
  QPointer<QToolButton> AdvLevelEditButton;

  void deleteWidget(QWidget* w)
  {
    if(w)
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
  }
};

//----------------------------------------------------------------------------
qtBaseView *
qtRootView::createViewWidget(smtk::common::ViewPtr dataObj,
                             QWidget* p, qtUIManager* uiman)
{
  return new qtRootView(dataObj, p, uiman);
}

//----------------------------------------------------------------------------
qtRootView::qtRootView(
  smtk::common::ViewPtr dataObj, QWidget* p, qtUIManager* uiman) :
  qtBaseView(dataObj, p, uiman)
{
  this->Internals = new qtRootViewInternals;
  this->ScrollArea = NULL;
  this->createWidget( );
}

//----------------------------------------------------------------------------
qtRootView::~qtRootView()
{
  this->Internals->clearWidgets();
  if(this->Internals->TabGroup)
    {
    delete this->Internals->TabGroup;
    }
  delete this->Internals;
  if ( this->ScrollArea )
    {
    delete this->ScrollArea;
    }
}
//----------------------------------------------------------------------------
void qtRootView::createWidget( )
{
  smtk::common::ViewPtr view = this->getObject();
  if(!view)
    {
    return;
    }
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
    bool val;
    int l;
    if (view->details().child(pos).contentsAsInt(l))
      {
      this->uiManager()->setMaxValueLabelLength(l);
      }
    }
  
  pos = view->details().findChild("MinValueLabelLength");
  if (pos != -1)
    {
    bool val;
    int l;
    if (view->details().child(pos).contentsAsInt(l))
      {
      this->uiManager()->setMinValueLabelLength(l);
      }
    }
  // Set up some properties to the uiManager
  
  this->Internals->clearWidgets();
  const System* attSys = this->uiManager()->attSystem();

  //first setup the advanced check box layout form
  //QHBoxLayout* advancedLayout = new QHBoxLayout();
  //advancedLayout->setMargin(10);
  this->Internals->AdvLevelCombo = new QComboBox(this->Widget);
  this->uiManager()->initAdvanceLevels(this->Internals->AdvLevelCombo);

  QLabel* advLevelLabel = new QLabel("Show Level:");
//  advLevelLabel->setFont(this->uiManager()->advancedFont());
  advLevelLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QToolButton* editButton = new QToolButton(this->Widget);
  editButton->setCheckable(true);
  QString resourceName(":/icons/attribute/lock.png");
  editButton->setFixedSize(QSize(20, 20));
  editButton->setIcon(QIcon(resourceName));
  editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  editButton->setToolTip("Edit access level");
  connect(editButton, SIGNAL(toggled(bool)),
        this, SLOT(showAdvanceLevelOverlay(bool)));
  this->Internals->AdvLevelLabel = advLevelLabel;
  this->Internals->AdvLevelEditButton = editButton;

  this->Internals->FilterByCheck = new QCheckBox(this->Widget);
  this->Internals->FilterByCheck->setText("Show by Category: ");
  this->Internals->FilterByCheck->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->Internals->ShowCategoryCombo = new QComboBox(this->Widget);
  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = attSys->categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    this->Internals->ShowCategoryCombo->addItem(it->c_str());
    }
  this->Internals->ShowCategoryCombo->setEnabled(false);
  QHBoxLayout* layout = new QHBoxLayout(this->Widget);
  layout->addWidget(editButton);
  layout->addWidget(advLevelLabel);
  layout->addWidget(this->Internals->AdvLevelCombo);
  layout->addWidget(this->Internals->FilterByCheck);
  layout->addWidget(this->Internals->ShowCategoryCombo);

  QObject::connect(this->Internals->FilterByCheck,
    SIGNAL(stateChanged(int)), this, SLOT(enableShowBy(int)));
  QObject::connect(this->Internals->ShowCategoryCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onShowCategory()));

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addLayout(layout);

  QObject::connect(this->Internals->AdvLevelCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onAdvanceLevelChanged(int)));
  this->showAdvanceLevel(this->uiManager()->advanceLevel());
}

//----------------------------------------------------------------------------
void qtRootView::onAdvanceLevelChanged(int levelIdx)
{
  int level = this->Internals->AdvLevelCombo->itemData(levelIdx).toInt();
  this->showAdvanceLevel(level);
}

//----------------------------------------------------------------------------
void qtRootView::showAdvanceLevel(int level)
{
  // update Root for smtk::view
  smtk::common::ViewPtr view = this->getObject();
  
  int currentTab = 0;
  if(this->Internals->TabGroup)
    {
    QTabWidget* selfW = static_cast<QTabWidget*>(
      this->Internals->TabGroup->widget());
    if(selfW)
      {
      QObject::disconnect(selfW, SIGNAL(currentChanged(int)),
        this, SLOT(updateViewUI(int)));
      if(this->Internals->TabGroup->childViews().count())
        {
        currentTab = selfW->currentIndex();
        }
      }
    delete this->Internals->TabGroup;
    }

  if(this->Widget)
    {
    this->parentWidget()->layout()->removeWidget(this->ScrollArea);
    delete this->Widget;
    delete this->ScrollArea;
    }

  this->Widget = new QFrame(this->parentWidget());

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());

  //create the scroll area on the tabs, so we don't make the
  //3d window super small
  this->ScrollArea = new QScrollArea();
  this->ScrollArea->setWidgetResizable(true);
  this->ScrollArea->setFrameShape(QFrame::NoFrame);
  this->ScrollArea->setObjectName("rootScrollArea");
  this->ScrollArea->setWidget( this->Widget );

  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  this->Internals->TabGroup = new qtGroupView(view, this->Widget,
    this->uiManager());
  this->initRootTabGroup();

  if(this->Internals->TabGroup->childViews().count())
    {
    QTabWidget* tabW = static_cast<QTabWidget*>(
      this->Internals->TabGroup->widget());
    if(tabW)
      {
      tabW->setCurrentIndex(currentTab);
      }
    }

  layout->addWidget(this->Internals->TabGroup->widget());

  //add the advanced layout, and the scroll area onto the
  //widgets to the frame
  parentlayout->addWidget(this->ScrollArea);
  QTabWidget* selfW = static_cast<QTabWidget*>(
    this->Internals->TabGroup->widget());
  if(selfW)
    {
    QObject::connect(selfW, SIGNAL(currentChanged(int)),
      this, SLOT(updateViewUI(int)));
    }

  if(level != this->advanceLevel())
    {
    this->Internals->AdvLevelCombo->blockSignals(true);
    for(int i=0; i<this->Internals->AdvLevelCombo->count(); i++)
      {
      int l = this->Internals->AdvLevelCombo->itemData(i).toInt();
      if(level == l)
        {
        this->Internals->AdvLevelCombo->setCurrentIndex(i);
        break;
        }
      }
    this->Internals->AdvLevelCombo->blockSignals(false);
    }
  this->uiManager()->setAdvanceLevel(level);
  if(this->advanceLevelVisible())
    {
    this->showAdvanceLevelOverlay(true);
    }
}

//----------------------------------------------------------------------------
void qtRootView::updateViewUI(int currentTab)
{
  qtBaseView* curSec = this->Internals->TabGroup->getChildView(currentTab);
  if(curSec)
    {
    curSec->updateUI();
    }
}
//----------------------------------------------------------------------------
void qtRootView::getChildView(const std::string &secType,
                              QList<qtBaseView*>& views)
{
  return this->Internals->TabGroup->getChildView(secType, views);
}
//----------------------------------------------------------------------------
qtGroupView* qtRootView::getRootGroup()
{
  return this->Internals->TabGroup;
}
//----------------------------------------------------------------------------
void qtRootView::initRootTabGroup()
{
  //if we are the root tab group we want to be display differently than the other tab groups
  QTabWidget *tab = dynamic_cast<QTabWidget*>(this->Internals->TabGroup->widget());
  if ( tab )
    {
    tab->setTabPosition(QTabWidget::East);

    //default icon size is style dependent. We will override this with a default
    //that icons can't be smaller than 32x32
    QSize ISize = tab->iconSize();
    if ( ISize.height() < 24 && ISize.width() < 24 )
      {
      tab->setIconSize( QSize(24,24) );
      }
    }
}
//----------------------------------------------------------------------------
void qtRootView::enableShowBy(int enable)
{
  this->Internals->ShowCategoryCombo->setEnabled(enable);
  this->filterByCategory();
}
//----------------------------------------------------------------------------
void qtRootView::onShowCategory()
{
  this->filterByCategory();
}
//----------------------------------------------------------------------------
void qtRootView::filterByCategory()
{
  QTabWidget* selfW = static_cast<QTabWidget*>(
    this->Internals->TabGroup->widget());
  if(selfW)
    {
    if(this->Internals->TabGroup->childViews().count())
      {
      int currentTab = selfW->currentIndex();
      this->updateViewUI(currentTab);
      }
    }
}

//----------------------------------------------------------------------------
int qtRootView::advanceLevel()
{
  return this->Internals->AdvLevelCombo->currentIndex();
}
//----------------------------------------------------------------------------
bool qtRootView::categoryEnabled()
{
  return this->Internals->FilterByCheck->isChecked();
}
//----------------------------------------------------------------------------
std::string qtRootView::currentCategory()
{
  return this->categoryEnabled() ?
    this->Internals->ShowCategoryCombo->currentText().toStdString() : "";
}

//----------------------------------------------------------------------------
void qtRootView::showAdvanceLevelOverlay(bool show)
{
  if(this->Internals->TabGroup)
    {
    this->Internals->TabGroup->showAdvanceLevelOverlay(show);
    }
  this->qtBaseView::showAdvanceLevelOverlay(show);
}
//----------------------------------------------------------------------------
void qtRootView::updateModelAssociation()
{
  this->Internals->TabGroup->updateModelAssociation();
}

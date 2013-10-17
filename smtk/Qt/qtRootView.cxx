/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "smtk/Qt/qtRootView.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtGroupView.h"
#include "smtk/view/Root.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QScrollArea>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtRootViewInternals
{
public:

  QPointer<QCheckBox> AdvancedCheck;
  QPointer<qtGroupView> TabGroup;
};

//----------------------------------------------------------------------------
qtRootView::qtRootView(
  smtk::view::RootPtr dataObj, QWidget* p) :
  qtBaseView(smtk::dynamic_pointer_cast<smtk::view::Base>(dataObj), p)
{
  this->Internals = new qtRootViewInternals;
  this->ScrollArea = NULL;
  this->createWidget( );
}

//----------------------------------------------------------------------------
qtRootView::~qtRootView()
{
  if(this->Internals->AdvancedCheck)
    {
    delete this->Internals->AdvancedCheck;
    }
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
  if(!this->getObject())
    {
    return;
    }

  if(this->Internals->AdvancedCheck)
    {
    delete this->Internals->AdvancedCheck;
    }

  //first setup the advanced check box layout form
  //QHBoxLayout* advancedLayout = new QHBoxLayout();
  //advancedLayout->setMargin(10);
  this->Internals->AdvancedCheck = new QCheckBox(this->Widget);
  this->Internals->AdvancedCheck->setText("Show Advanced");
  this->Internals->AdvancedCheck->setFont(
    qtUIManager::instance()->advancedFont());
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addWidget(this->Internals->AdvancedCheck);

  this->showAdvanced(0);

  QObject::connect(this->Internals->AdvancedCheck,
    SIGNAL(stateChanged(int)), this, SLOT(showAdvanced(int)));
}

//----------------------------------------------------------------------------
void qtRootView::showAdvanced(int checked)
{
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
  qtUIManager::instance()->setShowAdvanced(checked ? true : false);

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

  this->Internals->TabGroup = new qtGroupView(this->getObject(), this->Widget);
  qtUIManager::processGroupView(this->Internals->TabGroup);
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
void qtRootView::getChildView(
  smtk::view::Base::Type secType, QList<qtBaseView*>& views)
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

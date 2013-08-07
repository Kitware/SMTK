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

#include "smtk/Qt/qtRootSection.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtGroupSection.h"
#include "smtk/attribute/RootSection.h"

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
class qtRootSectionInternals
{
public:

  QPointer<QCheckBox> AdvancedCheck;
  QPointer<qtGroupSection> TabGroup;
};

//----------------------------------------------------------------------------
qtRootSection::qtRootSection(
  smtk::RootSectionPtr dataObj, QWidget* p) :
  qtSection(smtk::dynamicCastPointer<Section>(dataObj), p)
{
  this->Internals = new qtRootSectionInternals;
  this->ScrollArea = NULL;
  this->createWidget( );
}

//----------------------------------------------------------------------------
qtRootSection::~qtRootSection()
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
void qtRootSection::createWidget( )
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
void qtRootSection::showAdvanced(int checked)
{
  int currentTab = 0;

  if(this->Internals->TabGroup)
    {
    QTabWidget* selfW = static_cast<QTabWidget*>(
      this->Internals->TabGroup->widget());
    if(selfW)
      {
      QObject::disconnect(selfW, SIGNAL(currentChanged(int)),
        this, SLOT(updateSectionUI(int)));
      if(this->Internals->TabGroup->childSections().count())
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

  this->Internals->TabGroup = new qtGroupSection(this->getObject(), this->Widget);
  qtUIManager::processGroupSection(this->Internals->TabGroup);
  this->initRootTabGroup();

  if(this->Internals->TabGroup->childSections().count())
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
      this, SLOT(updateSectionUI(int)));
    }
}

//----------------------------------------------------------------------------
void qtRootSection::updateSectionUI(int currentTab)
{
  qtSection* curSec = this->Internals->TabGroup->getChildSection(currentTab);
  if(curSec)
    {
    curSec->updateUI();
    }
}
//----------------------------------------------------------------------------
void qtRootSection::getChildSection(
  smtk::attribute::Section::Type secType, QList<qtSection*>& sections)
{
  return this->Internals->TabGroup->getChildSection(secType, sections);
}
//----------------------------------------------------------------------------
qtGroupSection* qtRootSection::getRootGroup()
{
  return this->Internals->TabGroup;
}
//----------------------------------------------------------------------------
void qtRootSection::initRootTabGroup()
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
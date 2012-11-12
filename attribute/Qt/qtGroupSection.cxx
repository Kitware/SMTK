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

#include "qtGroupSection.h"

#include "qtUIManager.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QScrollArea>

using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtGroupSectionInternals
{
public:

  QPointer<QCheckBox> AdvancedCheck;
  QList<slctk::attribute::qtSection*> ChildSections;

};

//----------------------------------------------------------------------------
qtGroupSection::qtGroupSection(
  slctk::SectionPtr dataObj, QWidget* p) : qtSection(dataObj, p)
{
  this->Internals = new qtGroupSectionInternals;
  this->ScrollArea = NULL;
  this->createWidget( );

}

//----------------------------------------------------------------------------
qtGroupSection::~qtGroupSection()
{
  this->clearChildSections();
  if(this->Internals->AdvancedCheck)
    {
    delete this->Internals->AdvancedCheck;
    }
  delete this->Internals;

  if ( this->ScrollArea )
    {
    delete this->ScrollArea;
    }
}
//----------------------------------------------------------------------------
void qtGroupSection::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  if(this->Internals->AdvancedCheck)
    {
    delete this->Internals->AdvancedCheck;
    }
  if(this->Widget)
    {
    this->parentWidget()->layout()->removeWidget(this->ScrollArea);
    delete this->Widget;
    delete this->ScrollArea;
    }
    
  this->clearChildSections();
  this->Widget = new QFrame(this->parentWidget());
  
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());

  //first setup the advanced check box layout form
  //QHBoxLayout* advancedLayout = new QHBoxLayout();
  //advancedLayout->setMargin(10);
  this->Internals->AdvancedCheck = new QCheckBox(this->Widget);
  this->Internals->AdvancedCheck->setText("Show Advanced");
  this->Internals->AdvancedCheck->setFont(
    qtUIManager::instance()->advancedFont());

  //add the checkbox to the layout
  //advancedLayout->addWidget(this->Internals->AdvancedCheck);

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

  //add the advanced layout, and the scroll area onto the
  //widgets to the frame
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addWidget(this->Internals->AdvancedCheck);
  parentlayout->addWidget(this->ScrollArea);

  QObject::connect(this->Internals->AdvancedCheck,
    SIGNAL(stateChanged(int)), this, SLOT(showAdvanced(int)));
}

//----------------------------------------------------------------------------
void qtGroupSection::addChildSection(qtSection* child)
{
  if(!this->Internals->ChildSections.contains(child))
    {
    this->Internals->ChildSections.append(child);
    }
}
//----------------------------------------------------------------------------
QList<qtSection*>& qtGroupSection::childSections() const
{
  return this->Internals->ChildSections;
} 
//----------------------------------------------------------------------------
void qtGroupSection::clearChildSections()
{
  for(int i=0; i < this->Internals->ChildSections.count(); i++)
    {
    delete this->Internals->ChildSections.value(i);
    }
  this->Internals->ChildSections.clear();
}

//----------------------------------------------------------------------------
void qtGroupSection::showAdvanced(int checked)
{
  if(this->Widget)
    {
    this->parentWidget()->layout()->removeWidget(this->ScrollArea);
    delete this->Widget;
    delete this->ScrollArea;
    }

  this->clearChildSections();

  this->Widget = new QFrame(this->parentWidget());

  //create the scroll area on the tabs, so we don't make the
  //3d window super small
  this->ScrollArea = new QScrollArea();
  this->ScrollArea->setWidgetResizable(true);
  this->ScrollArea->setFrameShape(QFrame::NoFrame);
  this->ScrollArea->setObjectName("rootScrollArea");
  this->ScrollArea->setWidgetResizable( true );
  this->ScrollArea->setWidget( this->Widget );

  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  this->parentWidget()->layout()->addWidget( this->ScrollArea );

}

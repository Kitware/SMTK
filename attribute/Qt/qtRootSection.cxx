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

#include "qtRootSection.h"

#include "qtUIManager.h"
#include "attribute/RootSection.h"

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
class qtRootSectionInternals
{
public:

  QPointer<QCheckBox> AdvancedCheck;

};

//----------------------------------------------------------------------------
qtRootSection::qtRootSection(
  slctk::RootSectionPtr dataObj, QWidget* p) :
  qtGroupSection(slctk::dynamicCastPointer<Section>(dataObj), p)
{
  this->Internals = new qtRootSectionInternals;
  this->createWidget( );

}

//----------------------------------------------------------------------------
qtRootSection::~qtRootSection()
{
  if(this->Internals->AdvancedCheck)
    {
    delete this->Internals->AdvancedCheck;
    }
  delete this->Internals;
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
  
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());

  //first setup the advanced check box layout form
  //QHBoxLayout* advancedLayout = new QHBoxLayout();
  //advancedLayout->setMargin(10);
  this->Internals->AdvancedCheck = new QCheckBox(this->Widget);
  this->Internals->AdvancedCheck->setText("Show Advanced");
  this->Internals->AdvancedCheck->setFont(
    qtUIManager::instance()->advancedFont());

  //add the advanced layout, and the scroll area onto the
  //widgets to the frame
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addWidget(this->Internals->AdvancedCheck);

  QObject::connect(this->Internals->AdvancedCheck,
    SIGNAL(stateChanged(int)), this, SLOT(showAdvanced(int)));
}

//----------------------------------------------------------------------------
void qtRootSection::showAdvanced(int checked)
{
  int currentTab = 0;

  if(this->childSections().count())
    {
    QTabWidget* child = static_cast<QTabWidget*>(childSections().value(0)->widget());
    if(child)
      {
      currentTab = child->currentIndex();
      }
    }

  this->qtGroupSection::showAdvanced(checked);

  qtUIManager::instance()->setShowAdvanced(checked ? true : false);
  qtUIManager::instance()->processRootSection(this);

  if(this->childSections().count())
    {
    QTabWidget* child = static_cast<QTabWidget*>(childSections().value(0)->widget());
    if(child)
      {
      child->setCurrentIndex(currentTab);
      }
    }
}

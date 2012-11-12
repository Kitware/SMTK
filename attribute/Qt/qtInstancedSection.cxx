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

#include "qtInstancedSection.h"

#include "qtUIManager.h"
#include "qtAttribute.h"
#include "attribute/InstancedSection.h"
#include "attribute/Attribute.h"

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
class qtInstancedSectionInternals
{
public:
  qtInstancedSectionInternals()
    {
    this->ScrollArea = NULL;
    }
  QScrollArea *ScrollArea;
  QList< QPointer<qtAttribute> > AttInstances;
};

//----------------------------------------------------------------------------
qtInstancedSection::qtInstancedSection(
  slctk::SectionPtr dataObj, QWidget* p) : qtSection(dataObj, p)
{
  this->Internals = new qtInstancedSectionInternals;
  this->createWidget( );

}

//----------------------------------------------------------------------------
qtInstancedSection::~qtInstancedSection()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtInstancedSection::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  if(this->Widget)
    {
    this->parentWidget()->layout()->removeWidget(
      this->Internals->ScrollArea);
    delete this->Widget;
    delete this->Internals->ScrollArea;
    }

  this->Widget = new QFrame(this->parentWidget());

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());

  //create the scroll area on the tabs, so we don't make the
  //3d window super small
  this->Internals->ScrollArea = new QScrollArea();
  this->Internals->ScrollArea->setWidgetResizable(true);
  this->Internals->ScrollArea->setFrameShape(QFrame::NoFrame);
  this->Internals->ScrollArea->setObjectName("rootScrollArea");
  this->Internals->ScrollArea->setWidget( this->Widget );

  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  //add the advanced layout, and the scroll area onto the
  //widgets to the frame
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addWidget(this->Internals->ScrollArea);

  this->updateAttributeData();
}

//----------------------------------------------------------------------------
void qtInstancedSection::updateAttributeData()
{
  slctk::InstancedSectionPtr sec =
    slctk::dynamicCastPointer<InstancedSection>(this->getObject());
  if(!sec || !sec->numberOfInstances())
    {
    return;
    }
  this->Internals->AttInstances.clear();
  std::size_t i, n = sec->numberOfInstances();
  for (i = 0; i < n; i++)
    {
    qtAttribute* attInstance = new qtAttribute(sec->instance((int)i), this->widget());
    this->Widget->layout()->addWidget(attInstance->widget());
    this->Internals->AttInstances.push_back(attInstance);
    }
}

//----------------------------------------------------------------------------
void qtInstancedSection::showAdvanced(int checked)
{

}

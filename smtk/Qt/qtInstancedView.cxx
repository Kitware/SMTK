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

#include "smtk/Qt/qtInstancedView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtAttribute.h"
#include "smtk/view/Instanced.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QScrollArea>
#include <QMessageBox>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtInstancedViewInternals
{
public:
  qtInstancedViewInternals()
    {
    this->ScrollArea = NULL;
    }
  QScrollArea *ScrollArea;
  QList< QPointer<qtAttribute> > AttInstances;
};

//----------------------------------------------------------------------------
qtInstancedView::
qtInstancedView(smtk::view::BasePtr dataObj, QWidget* p, qtUIManager* uiman) :
  qtBaseView(dataObj, p, uiman)
{
  this->Internals = new qtInstancedViewInternals;
  this->createWidget( );

}

//----------------------------------------------------------------------------
qtInstancedView::~qtInstancedView()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtInstancedView::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());
  if(this->Widget)
    {
    if(parentlayout)
      {
      this->parentWidget()->layout()->removeWidget(this->Widget);
      }
    delete this->Widget;
    delete this->Internals->ScrollArea;
    }

  this->Widget = new QFrame(this->parentWidget());

  //create the scroll area on the tabs, so we don't make the
  //3d window super small
  this->Internals->ScrollArea = new QScrollArea();
  this->Internals->ScrollArea->setWidgetResizable(true);
  this->Internals->ScrollArea->setFrameShape(QFrame::NoFrame);
  this->Internals->ScrollArea->setObjectName("instancedViewScrollArea");
  this->Internals->ScrollArea->setWidget( this->Widget );

  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  //add the advanced layout, and the scroll area onto the
  //widgets to the frame
  if(parentlayout)
    {
    parentlayout->setAlignment(Qt::AlignTop);
    parentlayout->addWidget(this->Widget);
    }

  this->updateAttributeData();
}

//----------------------------------------------------------------------------
void qtInstancedView::updateAttributeData()
{
  smtk::view::InstancedPtr iview =
    smtk::dynamic_pointer_cast<smtk::view::Instanced>(this->getObject());
  if(!iview || !iview->numberOfInstances())
    {
    return;
    }
  foreach(qtAttribute* att, this->Internals->AttInstances)
    {
    this->Widget->layout()->removeWidget(att->widget());
    delete att->widget();
    }
  this->Internals->AttInstances.clear();
  std::size_t i, n = iview->numberOfInstances();
  for (i = 0; i < n; i++)
    {
    smtk::attribute::AttributePtr attobj = iview->instance(static_cast<int>(i));
    if(!attobj || attobj->numberOfItems()==0)
      {
      QMessageBox::warning(this->parentWidget(), tr("Instanced Attribute View"),
      tr("No attribute instance, or no items in the attribute instance!"));
      }
    else
      {
      qtAttribute* attInstance = new qtAttribute(attobj, this->widget(), this);
      if(attInstance)
        {
        this->Internals->AttInstances.push_back(attInstance);
        if(attInstance->widget())
          {
          this->Widget->layout()->addWidget(attInstance->widget());
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void qtInstancedView::showAdvanced(int /*checked*/)
{

}

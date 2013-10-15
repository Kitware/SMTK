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
#include "smtk/Qt/qtBaseView.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"

#include "smtk/view/Base.h"

#include <QPointer>
#include <QLayout>
#include <QWidget>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtBaseViewInternals
{
public:
  qtBaseViewInternals(smtk::view::BasePtr dataObject, QWidget* p)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  }
  ~qtBaseViewInternals()
  {
  }
  smtk::view::WeakBasePtr DataObject;
 QPointer<QWidget> ParentWidget;
};


//----------------------------------------------------------------------------
qtBaseView::qtBaseView(smtk::view::BasePtr dataObject, QWidget* p)
{
  this->Internals  = new qtBaseViewInternals(dataObject, p);
  this->Widget = NULL;
}

//----------------------------------------------------------------------------
qtBaseView::~qtBaseView()
{
  if (this->Internals)
    {
    if(this->Internals->ParentWidget && this->Widget
      && this->Internals->ParentWidget->layout())
      {
      this->Internals->ParentWidget->layout()->removeWidget(this->Widget);
      }
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
smtk::view::BasePtr qtBaseView::getObject()
{
  return this->Internals->DataObject.lock();
}

//----------------------------------------------------------------------------
QWidget* qtBaseView::parentWidget()
{
  return this->Internals->ParentWidget;
}
//----------------------------------------------------------------------------
void qtBaseView::getDefinitions(
  smtk::AttributeDefinitionPtr attDef,
  QList<smtk::AttributeDefinitionPtr>& defs)
{
  std::vector<smtk::AttributeDefinitionPtr> newdefs;
  Manager *attManager = attDef->manager();
  attManager->findAllDerivedDefinitions(attDef, true, newdefs);
  if(!attDef->isAbstract() && !defs.contains(attDef))
    {
    defs.push_back(attDef);
    }
  std::vector<smtk::AttributeDefinitionPtr>::iterator itDef;
  for (itDef=newdefs.begin(); itDef!=newdefs.end(); ++itDef)
    {
    if(!(*itDef)->isAbstract() && !defs.contains(*itDef))
      {
      defs.push_back(*itDef);
      }
    }
}

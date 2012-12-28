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
#include "qtSection.h"

#include "qtUIManager.h"
#include "attribute/Section.h"
#include "attribute/Definition.h"
#include "attribute/Manager.h"

#include <QPointer>
#include <QLayout>
#include <QWidget>

using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtSectionInternals
{
public:
  qtSectionInternals(slctk::SectionPtr dataObject, QWidget* p)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  }
  ~qtSectionInternals()
  {
  }
 slctk::WeakSectionPtr DataObject;
 QPointer<QWidget> ParentWidget;
};


//----------------------------------------------------------------------------
qtSection::qtSection(slctk::SectionPtr dataObject, QWidget* p)
{ 
  this->Internals  = new qtSectionInternals(dataObject, p); 
  this->Widget = NULL;
  //this->Internals->DataConnect = NULL;
  //this->createWidget();
}

//----------------------------------------------------------------------------
qtSection::~qtSection()
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
slctk::SectionPtr qtSection::getObject()
{
  return this->Internals->DataObject.lock();
}

//----------------------------------------------------------------------------
QWidget* qtSection::parentWidget()
{
  return this->Internals->ParentWidget;
}
//----------------------------------------------------------------------------
void qtSection::getDefinitions(
  slctk::AttributeDefinitionPtr attDef, 
  QList<slctk::AttributeDefinitionPtr>& defs)
{
  std::vector<slctk::AttributeDefinitionPtr> newdefs;
  Manager *attManager = attDef->manager();
  attManager->derivedDefinitions(attDef, newdefs);
  if(!attDef->isAbstract() && !defs.contains(attDef))
    {
    defs.push_back(attDef);
    }
  std::vector<slctk::AttributeDefinitionPtr>::iterator itDef;
  for (itDef=newdefs.begin(); itDef!=newdefs.end(); ++itDef)
    {
    if(!(*itDef)->isAbstract() && !defs.contains(attDef))
      {
      defs.push_back(*itDef);
      }
    }
}

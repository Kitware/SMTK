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

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"

#include "smtk/common/View.h"

#include <QPointer>
#include <QLayout>
#include <QWidget>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtBaseViewInternals
{
public:
  qtBaseViewInternals(smtk::common::ViewPtr dataObject, QWidget* p,
    qtUIManager* uiman)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  this->UIManager = uiman;
  this->FixedLabelWidth = uiman->maxValueLabelLength();
  }
  ~qtBaseViewInternals()
  {
  }
 smtk::common::ViewPtr DataObject;
 QPointer<QWidget> ParentWidget;
 QPointer<qtUIManager> UIManager;
 int FixedLabelWidth;
};


//----------------------------------------------------------------------------
qtBaseView::qtBaseView(smtk::common::ViewPtr dataObject, QWidget* p,
  qtUIManager* uiman)
{
  this->Internals  = new qtBaseViewInternals(dataObject, p, uiman);
  this->Widget = NULL;
  this->m_advOverlayVisible = false;
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
smtk::common::ViewPtr qtBaseView::getObject()
{
  return this->Internals->DataObject;
}

//----------------------------------------------------------------------------
QWidget* qtBaseView::parentWidget()
{
  return this->Internals->ParentWidget;
}
//----------------------------------------------------------------------------
void qtBaseView::getDefinitions(
  smtk::attribute::DefinitionPtr attDef,
  QList<smtk::attribute::DefinitionPtr>& defs)
{
  std::vector<smtk::attribute::DefinitionPtr> newdefs;
  System *attSystem = attDef->system();
  attSystem->findAllDerivedDefinitions(attDef, true, newdefs);
  if(!attDef->isAbstract() && !defs.contains(attDef))
    {
    defs.push_back(attDef);
    }
  std::vector<smtk::attribute::DefinitionPtr>::iterator itDef;
  for (itDef=newdefs.begin(); itDef!=newdefs.end(); ++itDef)
    {
    if(!(*itDef)->isAbstract() && !defs.contains(*itDef))
      {
      defs.push_back(*itDef);
      }
    }
}
//----------------------------------------------------------------------------
bool qtBaseView::displayItem(smtk::attribute::ItemPtr item)
{
  if (!item)
    {
    return false;
    }
  return this->uiManager()->passAdvancedCheck(item->advanceLevel()) &&
    this->uiManager()->passItemCategoryCheck(item->definition());
}

//----------------------------------------------------------------------------
qtUIManager* qtBaseView::uiManager()
{
  return this->Internals->UIManager;
}

//----------------------------------------------------------------------------
void qtBaseView::valueChanged(smtk::attribute::ItemPtr item)
{
  emit this->modified(item);
  this->uiManager()->onViewUIModified(this, item);
}

//----------------------------------------------------------------------------
int qtBaseView::fixedLabelWidth()
{
  return this->Internals->FixedLabelWidth;
}

//----------------------------------------------------------------------------
bool qtBaseView::setFixedLabelWidth(int w)
{
  w = std::min(w, this->uiManager()->maxValueLabelLength());
  w = std::max(w, this->uiManager()->minValueLabelLength());
  this->Internals->FixedLabelWidth = w;
  return false;
}

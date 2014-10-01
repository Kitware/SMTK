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

#include "smtk/view/Base.h"
#include "smtk/view/Root.h"

#include <QPointer>
#include <QLayout>
#include <QWidget>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtBaseViewInternals
{
public:
  qtBaseViewInternals(smtk::view::BasePtr dataObject, QWidget* p,
    qtUIManager* uiman)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  this->UIManager = uiman;
  smtk::view::RootPtr rs = uiman->attSystem()->rootView();
  this->FixedLabelWidth = rs->maxValueLabelLength();
  }
  ~qtBaseViewInternals()
  {
  }
 smtk::view::WeakBasePtr DataObject;
 QPointer<QWidget> ParentWidget;
 QPointer<qtUIManager> UIManager;
 int FixedLabelWidth;
};


//----------------------------------------------------------------------------
qtBaseView::qtBaseView(smtk::view::BasePtr dataObject, QWidget* p,
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
  smtk::view::RootPtr rs = this->uiManager()->attSystem()->rootView();
  w = std::min(w, rs->maxValueLabelLength());
  w = std::max(w, rs->minValueLabelLength());
  this->Internals->FixedLabelWidth = w;
  return false;
}

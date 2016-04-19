//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtPolygonEdgeOperationView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/common/View.h"

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
class qtPolygonEdgeOperationViewInternals
{
public:
  qtPolygonEdgeOperationViewInternals()
    {
    }
};

//----------------------------------------------------------------------------
qtBaseView *
qtPolygonEdgeOperationView::createViewWidget(const ViewInfo &info)
{
  qtPolygonEdgeOperationView *view = new qtPolygonEdgeOperationView(info);
  view->buildUI();
  return view;
}

//----------------------------------------------------------------------------
qtPolygonEdgeOperationView::
qtPolygonEdgeOperationView(const ViewInfo &info) :
  qtBaseView(info)
{
  this->Internals = new qtPolygonEdgeOperationViewInternals;

}

//----------------------------------------------------------------------------
qtPolygonEdgeOperationView::~qtPolygonEdgeOperationView()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::createWidget( )
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
      parentlayout->removeWidget(this->Widget);
      }
    delete this->Widget;
    }

  this->Widget = new QFrame(this->parentWidget());
  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  this->updateAttributeData();
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::updateAttributeData()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
    {
    return;
    }
}
//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::requestModelEntityAssociation()
{
}

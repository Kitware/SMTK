//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtModelPanel.h"

#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/Group.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Model.h"
#include "smtk/model/Manager.h"
#include "smtk/model/StringData.h"
#include "smtk/extension/qt/qtEntityItemDelegate.h"
#include "smtk/extension/qt/qtEntityItemModel.h"
#include "smtk/extension/qt/qtModelView.h"

#include <QPointer>

#include "ui_qtModelPanel.h"
namespace Ui { class qtModelPanel; }

// -----------------------------------------------------------------------------
namespace smtk {
  namespace model {


//-----------------------------------------------------------------------------
class qtModelPanel::qInternal : public Ui::qtModelPanel
{
public:
  QPointer<qtModelView> ModelView;

  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qtModelPanel::qtModelPanel(QWidget* p)
  : QWidget(p)
{
  this->Internal = new qInternal;
  this->Internal->ModelView = new qtModelView(p);
  this->Internal->setupUi(this);

  this->Internal->treeviewLayout->addWidget(this->Internal->ModelView);
  // signals/slots

  QObject::connect(this->Internal->ClearSelection,
    SIGNAL(clicked()), this, SLOT(onClearSelection()));
}

//-----------------------------------------------------------------------------
qtModelPanel::~qtModelPanel()
{
}

//-----------------------------------------------------------------------------
smtk::model::qtModelView* qtModelPanel::getModelView()
{
  return this->Internal->ModelView;
}

//-----------------------------------------------------------------------------
void qtModelPanel::onClearSelection()
{
  this->getModelView()->clearSelection();
}

  } // namespace model
} // namespace smtk

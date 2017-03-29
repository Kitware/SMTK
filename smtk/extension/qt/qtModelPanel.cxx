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

#include "smtk/extension/qt/qtEntityItemModel.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeSubphrases.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/Group.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/StringData.h"

#include <QPointer>

#include "ui_qtModelPanel.h"
namespace Ui { class qtModelPanel; }

// -----------------------------------------------------------------------------
namespace smtk {
  namespace extension {


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
  QObject::connect(this->Internal->comboBoxViewBy,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onViewTypeChanged()));
}

//-----------------------------------------------------------------------------
qtModelPanel::~qtModelPanel()
{
  delete this->Internal->ModelView;
  delete this->Internal;
}

//-----------------------------------------------------------------------------
smtk::extension::qtModelView* qtModelPanel::getModelView()
{
  return this->Internal->ModelView;
}

//-----------------------------------------------------------------------------
void qtModelPanel::onClearSelection()
{
  this->getModelView()->clearSelection();
}

//-----------------------------------------------------------------------------
void qtModelPanel::onViewTypeChanged()
{
  int type = this->Internal->comboBoxViewBy->currentIndex();
  qtModelPanel::enumTreeView enType = (type == 0) ?
    qtModelPanel::VIEW_BY_TOPOLOGY : qtModelPanel::VIEW_BY_ENTITY_LIST;
  QPointer<smtk::extension::QEntityItemModel> qmodel = this->getModelView()->getModel();
  if(!qmodel->manager())
    return;
  this->resetView(enType, qmodel->manager());
}

//-----------------------------------------------------------------------------
void qtModelPanel::resetView(qtModelPanel::enumTreeView enType,
                             smtk::model::ManagerPtr modelMgr)
{
  this->Internal->comboBoxViewBy->blockSignals(true);
  smtk::model::SubphraseGeneratorPtr spg;
  if(enType == qtModelPanel::VIEW_BY_TOPOLOGY)
    {
    spg = smtk::model::SimpleModelSubphrases::create();
    this->Internal->comboBoxViewBy->setCurrentIndex(0);
    }
  else if(enType == qtModelPanel::VIEW_BY_ENTITY_LIST)
    {
    spg = smtk::model::EntityTypeSubphrases::create();
    this->Internal->comboBoxViewBy->setCurrentIndex(1);
    }
  this->Internal->comboBoxViewBy->blockSignals(false);

  if(!spg)
    return;

  smtk::model::BitFlags mask = smtk::model::SESSION;
  smtk::model::EntityRefs cursors;
  smtk::model::EntityRef::EntityRefsFromUUIDs(
    cursors, modelMgr, modelMgr->entitiesMatchingFlags(mask, true));

  spg->setDirectLimit(-1);
  spg->setSkipAttributes(true);
  spg->setSkipProperties(true);

  qtModelView* modelview = this->getModelView();
  QPointer<smtk::extension::QEntityItemModel> qmodel = modelview->getModel();
  qmodel->clear();

  qmodel->setRoot(
    smtk::model::EntityListPhrase::create()
      ->setup(cursors)
      ->setDelegate(spg));// set the subphrase generator

  modelview->expandAllModels();

}

  } // namespace extension
} // namespace smtk

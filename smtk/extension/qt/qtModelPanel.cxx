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

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/Group.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/StringData.h"

#include <QPointer>

#include "ui_qtModelPanel.h"
namespace Ui
{
class qtModelPanel;
}

namespace smtk
{
namespace extension
{

class qtModelPanel::qInternal : public Ui::qtModelPanel
{
public:
  QPointer<qtModelView> ModelView;

  qInternal() = default;
};

qtModelPanel::qtModelPanel(QWidget* p)
  : QWidget(p)
{
  this->Internal = new qInternal;
  this->Internal->ModelView = new qtModelView(p);
  this->Internal->setupUi(this);

  this->Internal->treeviewLayout->addWidget(this->Internal->ModelView);
  // signals/slots

  QObject::connect(
    this->Internal->ClearSelection, SIGNAL(clicked()), this, SLOT(onClearSelection()));
  QObject::connect(this->Internal->comboBoxViewBy, SIGNAL(currentIndexChanged(int)), this,
    SLOT(onViewTypeChanged()));
}

qtModelPanel::~qtModelPanel()
{
  delete this->Internal->ModelView;
  delete this->Internal;
}

smtk::extension::qtModelView* qtModelPanel::getModelView()
{
  return this->Internal->ModelView;
}

void qtModelPanel::onClearSelection()
{
  this->getModelView()->clearSelection();
}

void qtModelPanel::onViewTypeChanged()
{
  int type = this->Internal->comboBoxViewBy->currentIndex();
  qtModelPanel::enumTreeView enType =
    (type == 0) ? qtModelPanel::VIEW_BY_TOPOLOGY : qtModelPanel::VIEW_BY_ENTITY_LIST;
  (void)enType;
}

void qtModelPanel::resetView(
  qtModelPanel::enumTreeView /*unused*/, smtk::model::ResourcePtr /*unused*/)
{
}

} // namespace extension
} // namespace smtk

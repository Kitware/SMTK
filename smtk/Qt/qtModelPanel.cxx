#include "smtk/Qt/qtModelPanel.h"

#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/StringData.h"
#include "smtk/Qt/qtEntityItemDelegate.h"
#include "smtk/Qt/qtEntityItemModel.h"
#include "smtk/Qt/qtModelView.h"

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

  QObject::connect(this->Internal->AddDomainset,
    SIGNAL(clicked()), this, SLOT(onAddDomainset()));
  QObject::connect(this->Internal->RemoveDomainset,
    SIGNAL(clicked()), this, SLOT(onRemoveDomainset()));
  QObject::connect(this->Internal->AddBC,
    SIGNAL(clicked()), this, SLOT(onAddBC()));
  QObject::connect(this->Internal->RemoveBC,
    SIGNAL(clicked()), this, SLOT(onRemoveBC()));
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
void qtModelPanel::onAddDomainset()
{

}

//-----------------------------------------------------------------------------
void qtModelPanel::onRemoveDomainset()
{
}

//-----------------------------------------------------------------------------
void qtModelPanel::onAddBC()
{
  QEntityItemModel* qmodel = this->getModelView()->getModel();
  smtk::model::StoragePtr pstore = qmodel->storage();
  // bgroup.addEntities(entities);
  smtk::util::UUIDs ents = pstore->entitiesMatchingFlags(MODEL_ENTITY, false);
  if(!ents.empty())
    {
    smtk::model::ModelEntity me(pstore, *ents.begin());
    GroupEntity bgroup = pstore->addGroup(
      smtk::model::MODEL_BOUNDARY, "BC Group");
    me.addGroup(bgroup);
    this->Internal->ModelView->repaint();
    }
}

//-----------------------------------------------------------------------------
void qtModelPanel::onRemoveBC()
{
}

  } // namespace model
} // namespace smtk

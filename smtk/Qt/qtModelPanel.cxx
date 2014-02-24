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
  QEntityItemModel* qmodel = this->getModelView()->getModel();
  smtk::model::StoragePtr pstore = qmodel->storage();
  ModelEntities models;
  smtk::model::Cursor::CursorsFromUUIDs(
    models,
    pstore,
    pstore->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));

  if(!models.empty())
    {
    GroupEntity ds = pstore->addGroup(
      DIMENSION_3, "Domain Set");
    models.begin()->addGroup(ds);
    std::cout << "Added " << ds.name() << " to " << models.begin()->name() << "\n";
    }
}

//-----------------------------------------------------------------------------
void qtModelPanel::onRemoveDomainset()
{
  DescriptivePhrase* dp = this->Internal->ModelView->currentItem();
  if(dp && dp->relatedEntity().isGroupEntity())
    {
    QEntityItemModel* qmodel = this->getModelView()->getModel();
    qmodel->storage()->erase(dp->relatedEntityId());
    }
}

//-----------------------------------------------------------------------------
void qtModelPanel::onAddBC()
{
  QEntityItemModel* qmodel = this->getModelView()->getModel();
  smtk::model::StoragePtr pstore = qmodel->storage();
  ModelEntities models;
  smtk::model::Cursor::CursorsFromUUIDs(
    models,
    pstore,
    pstore->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));

  if(!models.empty())
    {
    GroupEntity bgroup = pstore->addGroup(
      DIMENSION_2, "BC Group");
    models.begin()->addGroup(bgroup);
    std::cout << "Added " << bgroup.name() << " to " << models.begin()->name() << "\n";
    }
}

//-----------------------------------------------------------------------------
void qtModelPanel::onRemoveBC()
{
  DescriptivePhrase* dp = this->Internal->ModelView->currentItem();
  if(dp && dp->relatedEntity().isGroupEntity())
    {
    QEntityItemModel* qmodel = this->getModelView()->getModel();
    qmodel->storage()->erase(dp->relatedEntityId());
    }
}

  } // namespace model
} // namespace smtk

#include "smtk/Qt/testing/ModelBrowser.h"
#include "smtk/Qt/qtEntityItemModel.h"
#include "smtk/Qt/qtEntityItemDelegate.h"

#include "smtk/Qt/testing/ui_ModelBrowser.h"

#include "smtk/model/GroupEntity.h"
#include "smtk/model/ModelEntity.h"

#include <QtGui/QPushButton>
#include <QtGui/QTreeView>

namespace Ui { class qtAttributeAssociation; }

using namespace smtk::model;

class ModelBrowser::Internals : public Ui::ModelBrowser
{
};

ModelBrowser::ModelBrowser(QWidget* p) :
  QWidget(p)
{
  this->m_p = new ModelBrowser::Internals;
  this->m_p->setupUi(this);
  QObject::connect(
    this->m_p->addGroupButton, SIGNAL(clicked()),
    this, SLOT(addGroup())
  );
}

ModelBrowser::~ModelBrowser()
{
  delete this->m_p;
}

QTreeView* ModelBrowser::tree() const
{
  return this->m_p->modelTree;
}

void ModelBrowser::setup(
  smtk::model::StoragePtr storage,
  smtk::model::QEntityItemModel* qmodel,
  smtk::model::QEntityItemDelegate* qdelegate,
  smtk::model::DescriptivePhrasePtr root)
{
  this->m_storage = storage;
  qmodel->setRoot(root);
  this->m_p->modelTree->setModel(qmodel); // Must come after qmodel->setRoot()!
  this->m_p->modelTree->setItemDelegate(qdelegate);
}

void ModelBrowser::addGroup()
{
  GroupEntity newGroup = this->m_storage->addGroup(0, "New Group");
  ModelEntities models;
  smtk::model::Cursor::CursorsFromUUIDs(
    models,
    this->m_storage,
    this->m_storage->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));
  if (!models.empty())
    {
    models.begin()->addGroup(newGroup);
    std::cout << "Added " << newGroup.name() << " to " << models.begin()->name() << "\n";
    }
}

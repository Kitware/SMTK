#include "smtk/Qt/testing/ModelBrowser.h"
#include "smtk/Qt/qtEntityItemModel.h"
#include "smtk/Qt/qtEntityItemDelegate.h"

#include "smtk/Qt/testing/ui_ModelBrowser.h"

#include "smtk/model/GroupEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"

#include <QtGui/QPushButton>
#include <QtGui/QTreeView>

namespace Ui { class qtAttributeAssociation; }

using namespace smtk::model;

class ModelBrowser::Internals : public Ui::ModelBrowser
{
public:
  smtk::model::QEntityItemModel* qmodel;
  smtk::model::QEntityItemDelegate* qdelegate;
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
  QObject::connect(
    this->m_p->removeFromGroupButton, SIGNAL(clicked()),
    this, SLOT(removeFromGroup())
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
  this->m_p->qmodel = qmodel;
  this->m_p->qdelegate = qdelegate;
  QObject::connect(
    this->m_p->modelTree->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
    this, SLOT(updateButtonStates(const QModelIndex&, const QModelIndex&))
  );
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

void ModelBrowser::removeFromGroup()
{
  QModelIndex qidx = this->m_p->modelTree->currentIndex();
  GroupEntity group;
  if ((group = this->groupParentOfIndex(qidx)).isValid())
    {
    DescriptivePhrase* phrase = this->m_p->qmodel->getItem(qidx);
    if (phrase)
      {
      // Removing from the group emits a signal that
      // m_p->qmodel listens for, causing m_p->modelTree redraw.
      group.removeEntity(phrase->relatedEntity());
      }
    }
}

void ModelBrowser::updateButtonStates(const QModelIndex& curr, const QModelIndex&)
{
  this->m_p->removeFromGroupButton->setEnabled(
    groupParentOfIndex(curr).isValid());
}

/**\brief Does \a qidx refer to an entity that is displayed as the child of a group?
  *
  * Note that a group (EntityPhrase with a Cursor whose isGroup() is true)
  * may contain an EntityListPhrase, each entry of which is in the group.
  * We must test for this 1 level of indirection as well as for direct
  * children.
  */
smtk::model::GroupEntity ModelBrowser::groupParentOfIndex(const QModelIndex& qidx)
{
  smtk::model::GroupEntity group;
  DescriptivePhrase* phrase = this->m_p->qmodel->getItem(qidx);
  if (phrase)
    {
    EntityPhrase* ephrase = dynamic_cast<EntityPhrase*>(phrase);
    if (ephrase && ephrase->relatedEntity().isValid())
      {
      phrase = ephrase->parent().get();
      if (phrase)
        {
        ephrase = dynamic_cast<EntityPhrase*>(phrase);
        if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::GroupEntity>()).isValid())
          return group; // direct child of a GroupEntity's summary phrase.
        EntityListPhrase* lphrase = dynamic_cast<EntityListPhrase*>(phrase);
        if (lphrase)
          {
          ephrase = dynamic_cast<EntityPhrase*>(lphrase->parent().get());
          if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::GroupEntity>()).isValid())
            return group; // member of a list inside a GroupEntity's summary.
          }
        }
      }
    }
  return group;
}

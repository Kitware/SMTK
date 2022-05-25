//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/examples/cxx/ModelBrowser.h"

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/extension/qt/examples/cxx/ui_ModelBrowser.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseContent.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/model/Group.h"
#include "smtk/model/Model.h"

#include <QPushButton>
#include <QTreeView>

namespace Ui
{
class qtAttributeAssociation;
}

using namespace smtk::model;

class ModelBrowser::Internals : public Ui::ModelBrowser
{
public:
  smtk::extension::qtDescriptivePhraseModel* qmodel;
  smtk::extension::qtDescriptivePhraseDelegate* qdelegate;
};

ModelBrowser::ModelBrowser(QWidget* p)
  : QWidget(p)
{
  m_p = new ModelBrowser::Internals;
  m_p->setupUi(this);
  QObject::connect(m_p->addGroupButton, SIGNAL(clicked()), this, SLOT(addGroup()));
  QObject::connect(m_p->removeFromGroupButton, SIGNAL(clicked()), this, SLOT(removeFromGroup()));
  QObject::connect(m_p->addToGroupButton, SIGNAL(clicked()), this, SLOT(addToGroup()));
}

ModelBrowser::~ModelBrowser()
{
  delete m_p;
}

QTreeView* ModelBrowser::tree() const
{
  return m_p->modelTree;
}

void ModelBrowser::setup(
  smtk::resource::ManagerPtr manager,
  smtk::extension::qtDescriptivePhraseModel* qmodel,
  smtk::extension::qtDescriptivePhraseDelegate* qdelegate)
{
  m_manager = manager;
  m_p->modelTree->setItemDelegate(qdelegate);
  m_p->qmodel = qmodel;
  m_p->modelTree->setModel(m_p->qmodel);
  m_p->qdelegate = qdelegate;
  // NB: If we want the ModelBrowser widget to handle badge
  //     clicks, we would need to install an event filter
  //     on m_p->modelTree->viewport() and call
  //     qtDescriptivePhraseDelegate::processBadgeClick on
  //     mouse button-press events from the event filter.
  QObject::connect(
    m_p->modelTree->selectionModel(),
    SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
    this,
    SLOT(updateButtonStates(const QModelIndex&, const QModelIndex&)));
}

void ModelBrowser::addGroup()
{
  /*
  Group newGroup = m_manager->addGroup(0, "New Group");
  Models models;
  smtk::model::EntityRef::EntityRefsFromUUIDs(
    models, m_manager, m_manager->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));
  if (!models.empty())
  {
    models.begin()->addGroup(newGroup);
    std::cout << "Added " << newGroup.name() << " to " << models.begin()->name() << "\n";
  }
  */
}

// Add the entity under the entityref to the first group (ordered by UUID)
void ModelBrowser::addToGroup()
{
  /*
  QModelIndex qidx = m_p->modelTree->currentIndex();
  Group group;
  EntityRef item;
  Groups groups;
  EntityRef::EntityRefsFromUUIDs(
    groups, m_manager, m_manager->entitiesMatchingFlags(smtk::model::GROUP_ENTITY));
  if (groups.empty())
    return;

  group = *groups.begin();

  // Only keep the phrase alive as long as we must.
  {
    auto phrase = qidx.data(smtk::extension::qtDescriptivePhraseModel::PhrasePtrRole).value<DescriptivePhrasePtr>();
    if (!phrase)
      return;

    item = phrase->relatedEntity();
  }

  if (!item.isValid())
    return;

  // Now that our shared pointer keeping "phrase" alive is gone,
  // remove the entity from the group.
  if (group.isValid())
  {
    std::cout << "Adding " << item.name() << " to " << group.name() << "\n";
    group.addEntity(item);
  }
  */
}

void ModelBrowser::removeFromGroup()
{
  /*
  QModelIndex qidx = m_p->modelTree->currentIndex();
  Group group;
  if ((group = this->groupParentOfIndex(qidx)).isValid())
  {
    EntityRef relEnt;
    {
      auto phrase = qidx.data(smtk::extension::qtDescriptivePhraseModel::PhrasePtrRole).value<DescriptivePhrasePtr>();
      if (phrase)
        relEnt = phrase->relatedEntity();
    }
    // Now that our shared pointer keeping "phrase" alive is gone,
    // remove the entity from the group.
    if (relEnt.isValid())
    {
      // Deselect the current row, select another.
      // This helps un-confuse Qt's treeview.
      QModelIndex sidx = qidx.parent();
      int nsiblings = sidx.model()->rowCount(sidx);
      if (nsiblings)
      {
        if (qidx.row() < nsiblings - 1)
          sidx = qidx.sibling(qidx.row() + 1, 0);
        else if (qidx.row() > 0)
          sidx = qidx.sibling(qidx.row() - 1, 0);
      }
      if (sidx.model() != qidx.model())
      {
        std::cout << "Erp! Models differ: q " << qidx.model() << " s " << sidx.model() << "\n";
      }
      if (sidx.parent() != qidx.parent())
      {
        std::cout << "Erp! Model parents"
                  << "   s\""
                  << sidx.model()
                       ->data(sidx.parent(), smtk::extension::qtDescriptivePhraseModel::TitleTextRole)
                       .toString()
                       .toStdString()
                  << "\" q\""
                  << sidx.model()
                       ->data(qidx.parent(), smtk::extension::qtDescriptivePhraseModel::TitleTextRole)
                       .toString()
                       .toStdString()
                  << "\" differ\n";
      }
      m_p->modelTree->selectionModel()->select(
        sidx, QItemSelectionModel::Columns | QItemSelectionModel::SelectCurrent);
      // Removing from the group emits a signal that
      // m_p->qmodel listens for, causing m_p->modelTree redraw.
      group.removeEntity(relEnt);
    }
  }
  */
}

void ModelBrowser::updateButtonStates(const QModelIndex& curr, const QModelIndex&)
{
  (void)curr;
  // m_p->removeFromGroupButton->setEnabled(groupParentOfIndex(curr).isValid());
}

/**\brief Does \a qidx refer to an entity that is displayed as the child of a group?
  *
  * Note that a group (EntityPhrase with a EntityRef whose isGroup() is true)
  * may contain an EntityListPhrase, each entry of which is in the group.
  * We must test for this 1 level of indirection as well as for direct
  * children.
  */
smtk::model::Group ModelBrowser::groupParentOfIndex(const QModelIndex& qidx)
{
  (void)qidx;
  smtk::model::Group group;
  /*
  auto phrase = qidx.data(smtk::extension::qtDescriptivePhraseModel::PhrasePtrRole).value<DescriptivePhrasePtr>();
  if (phrase)
  {
    EntityPhrasePtr ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(phrase);
    if (ephrase && ephrase->relatedEntity().isValid())
    {
      phrase = ephrase->parent();
      if (phrase)
      {
        ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(phrase);
        if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::Group>()).isValid())
          return group; // direct child of a Group's summary phrase.
        EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(phrase);
        if (lphrase)
        {
          ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(lphrase->parent());
          if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::Group>()).isValid())
            return group; // member of a list inside a Group's summary.
        }
      }
    }
  }
  */
  return group;
}

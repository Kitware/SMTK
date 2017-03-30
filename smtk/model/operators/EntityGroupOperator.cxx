//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/EntityGroupOperator.h"

#include "smtk/model/Session.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/EntityGroupOperator_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

bool EntityGroupOperator::ableToOperate()
{
  smtk::model::Model model;
  bool ableToOperate = (this->ensureSpecification() &&
    (model = this->specification()->findModelEntity("model")->value().as<Model>()).isValid());

  if (!ableToOperate)
    return ableToOperate;
  // check data for create, edit and remove operation
  std::string optype = this->specification()->findString("Operation")->value();
  if (optype == "Modify")
  {
    smtk::attribute::ModelEntityItemPtr modgrpItem =
      this->specification()->findModelEntity("modify cell group");
    smtk::attribute::ModelEntityItemPtr addItem =
      this->specification()->findModelEntity("cell to add");
    smtk::attribute::ModelEntityItemPtr removeItem =
      this->specification()->findModelEntity("cell to remove");
    ableToOperate = (modgrpItem->isValid() && (addItem || removeItem));
  }
  else if (optype == "Remove")
  {
    smtk::attribute::ModelEntityItemPtr remgrpItem =
      this->specification()->findModelEntity("remove cell group");
    ableToOperate = (remgrpItem->isValid());
  }
  return ableToOperate;
}

smtk::model::OperatorResult EntityGroupOperator::operateInternal()
{
  // pre processing the data
  smtk::model::ManagerPtr manager = this->manager();
  // ableToOperate should have verified that model(s) are set
  smtk::attribute::StringItem::Ptr optypeItem = this->specification()->findString("Operation");
  std::string optype = optypeItem->value();
  smtk::model::Model model =
    this->specification()->findModelEntity("model")->value().as<smtk::model::Model>();

  smtk::model::Group bGroup;
  smtk::model::EntityRefArray modGroups;
  smtk::model::EntityRefArray remGroups;
  bGroup.setManager(manager);
  bool ok = false;
  BitFlags mask(0);

  if (optype == "Create")
  {
    std::string gName = this->specification()->findString("group name")->value();
    // initialize the group
    bGroup = manager->addGroup(0, gName);

    // check vertex, edge, face and volumes are enabled or not
    bool vertexFlag = this->findVoid("Vertex")->isEnabled();
    bool edgeFlag = this->findVoid("Edge")->isEnabled();
    bool faceFlag = this->findVoid("Face")->isEnabled();
    bool volumeFlag = this->findVoid("Volume")->isEnabled();
    if (vertexFlag)
      mask |= smtk::model::VERTEX;
    if (edgeFlag)
      mask |= smtk::model::EDGE;
    if (faceFlag)
      mask |= smtk::model::FACE;
    if (volumeFlag)
      mask |= smtk::model::VOLUME;
    bGroup.setMembershipMask(mask);

    smtk::attribute::ModelEntityItemPtr addItem =
      this->specification()->findModelEntity("cell to add");
    if (addItem)
    {
      for (std::size_t idx = 0; idx < addItem->numberOfValues(); ++idx)
      {
        bGroup.addEntity(addItem->value(idx));
      }
    }

    bGroup.setName(gName);
    model.addGroup(bGroup);
    ok = bGroup.isValid();
    std::cout << "new group: " << bGroup.name() << " id: " << bGroup.entity() << std::endl;
  }
  else if (optype == "Remove")
  {
    smtk::attribute::ModelEntityItemPtr remgrpItem =
      this->specification()->findModelEntity("remove cell group");
    for (std::size_t idx = 0; idx < remgrpItem->numberOfValues(); idx++)
    {
      // get rid of the group from manager
      smtk::model::EntityRef grpRem = remgrpItem->value(idx);
      model.removeGroup(grpRem.as<smtk::model::Group>());
      manager->erase(grpRem);
      std::cout << "Removed " << grpRem.name() << " to " << model.name() << "\n";
      remGroups.push_back(grpRem);
    }

    ok = remGroups.size() == 0 ? false : true;
  }
  else if (optype == "Modify")
  {
    smtk::model::EntityRef modifyEntity =
      this->specification()->findModelEntity("modify cell group")->value();
    smtk::model::Group modifyGroup = modifyEntity.as<smtk::model::Group>();

    // check vertex, edge, face and volumes are enabled or not
    bool vertexFlag = this->findVoid("Vertex")->isEnabled();
    bool edgeFlag = this->findVoid("Edge")->isEnabled();
    bool faceFlag = this->findVoid("Face")->isEnabled();
    bool volumeFlag = this->findVoid("Volume")->isEnabled();

    if (vertexFlag)
      mask |= smtk::model::VERTEX;
    if (edgeFlag)
      mask |= smtk::model::EDGE;
    if (faceFlag)
      mask |= smtk::model::FACE;
    if (volumeFlag)
      mask |= smtk::model::VOLUME;
    modifyGroup.setMembershipMask(mask);

    // start to process cell to add
    smtk::attribute::ModelEntityItemPtr addItem =
      this->specification()->findModelEntity("cell to add");
    if (addItem)
    {
      for (std::size_t idx = 0; idx < addItem->numberOfValues(); idx++)
      {
        smtk::model::EntityRef addEntity = addItem->value(idx);
        modifyGroup.addEntity(addEntity);
      }
    }

    // start to process cell to remove
    smtk::attribute::ModelEntityItemPtr removeItem =
      this->specification()->findModelEntity("cell to remove");
    if (removeItem)
    {
      for (std::size_t idx = 0; idx < removeItem->numberOfValues(); idx++)
      {
        smtk::model::EntityRef rmEntity = removeItem->value(idx);
        modifyGroup.removeEntity(rmEntity);
      }
    }
    modGroups.push_back(modifyGroup);
    ok = modGroups.size() == 0 ? false : true;
  }

  OperatorResult result = this->createResult(ok ? OPERATION_SUCCEEDED : OPERATION_FAILED);
  if (ok)
  {
    //create and modify
    if (bGroup.isValid())
    {
      // Return the created or modified group
      if (optype == "Create")
      {
        this->addEntityToResult(result, bGroup, CREATED);
      }
    }
    else if (optype == "Modify")
    {
      this->addEntitiesToResult(result, modGroups, MODIFIED);
    }
    else if (optype == "Remove")
    {
      this->addEntitiesToResult(result, remGroups, EXPUNGED);
    }
  }
  return result;
}

} //namespace model
} // namespace smtk

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::EntityGroupOperator, entity_group,
  "entity group", EntityGroupOperator_xml, smtk::model::Session);

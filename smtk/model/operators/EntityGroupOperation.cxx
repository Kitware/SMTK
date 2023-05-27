//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/EntityGroupOperation.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/operators/EntityGroupOperation_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

bool EntityGroupOperation::ableToOperate()
{
  auto assoc = this->parameters()->associations();
  smtk::model::Model model = assoc->valueAs<smtk::model::Entity>();
  bool ableToOperate = (this->Superclass::ableToOperate() && model.isValid());

  if (!ableToOperate)
    return ableToOperate;
  // check data for create, edit and remove operation
  std::string optype = this->parameters()->findString("Operation")->value();
  if (optype == "Modify")
  {
    auto modgrpItem = this->parameters()->findComponent("modify cell group");
    auto addItem = this->parameters()->findComponent("cell to add");
    auto removeItem = this->parameters()->findComponent("cell to remove");
    ableToOperate = (modgrpItem->isValid() && (addItem || removeItem));
  }
  else if (optype == "Remove")
  {
    auto remgrpItem = this->parameters()->findComponent("remove cell group");
    ableToOperate = (remgrpItem->isValid());
  }
  return ableToOperate;
}

EntityGroupOperation::Result EntityGroupOperation::operateInternal()
{
  // pre processing the data
  auto modelItem = this->parameters()->associations();
  smtk::model::Model model = modelItem->valueAs<smtk::model::Entity>();
  smtk::model::Resource::Ptr resource = model.resource();
  smtk::attribute::StringItem::Ptr optypeItem = this->parameters()->findString("Operation");
  std::string optype = optypeItem->value();

  smtk::model::Group bGroup;
  smtk::model::EntityRefArray modGroups;
  smtk::model::EntityRefArray remGroups;
  bGroup.setResource(resource);
  bool ok = false;
  BitFlags mask(0);

  if (optype == "Create")
  {
    std::string gName = this->parameters()->findString("group name")->value();
    // initialize the group
    bGroup = resource->addGroup(0, gName);

    // check vertex, edge, face and volumes are enabled or not
    bool vertexFlag = this->parameters()->findVoid("Vertex")->isEnabled();
    bool edgeFlag = this->parameters()->findVoid("Edge")->isEnabled();
    bool faceFlag = this->parameters()->findVoid("Face")->isEnabled();
    bool volumeFlag = this->parameters()->findVoid("Volume")->isEnabled();
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
      this->parameters()->findModelEntity("cell to add");
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
      this->parameters()->findModelEntity("remove cell group");
    for (std::size_t idx = 0; idx < remgrpItem->numberOfValues(); idx++)
    {
      // get rid of the group from resource
      smtk::model::EntityRef grpRem = remgrpItem->value(idx);
      model.removeGroup(grpRem.as<smtk::model::Group>());
      resource->erase(grpRem);
      std::cout << "Removed " << grpRem.name() << " to " << model.name() << "\n";
      remGroups.push_back(grpRem);
    }

    ok = !remGroups.empty();
  }
  else if (optype == "Modify")
  {
    smtk::model::EntityRef modifyEntity =
      this->parameters()->findModelEntity("modify cell group")->value();
    smtk::model::Group modifyGroup = modifyEntity.as<smtk::model::Group>();

    // check vertex, edge, face and volumes are enabled or not
    bool vertexFlag = this->parameters()->findVoid("Vertex")->isEnabled();
    bool edgeFlag = this->parameters()->findVoid("Edge")->isEnabled();
    bool faceFlag = this->parameters()->findVoid("Face")->isEnabled();
    bool volumeFlag = this->parameters()->findVoid("Volume")->isEnabled();

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
      this->parameters()->findModelEntity("cell to add");
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
      this->parameters()->findModelEntity("cell to remove");
    if (removeItem)
    {
      for (std::size_t idx = 0; idx < removeItem->numberOfValues(); idx++)
      {
        smtk::model::EntityRef rmEntity = removeItem->value(idx);
        modifyGroup.removeEntity(rmEntity);
      }
    }
    modGroups.push_back(modifyGroup);
    ok = !modGroups.empty();
  }

  Result result = this->createResult(
    ok ? smtk::operation::Operation::Outcome::SUCCEEDED
       : smtk::operation::Operation::Outcome::FAILED);
  if (ok)
  {
    //create and modify
    if (bGroup.isValid())
    {
      // Return the created or modified group
      if (optype == "Create")
      {
        smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
        createdItem->appendValue(bGroup.component());
      }
    }
    else if (optype == "Modify")
    {
      smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
      for (auto& m : modGroups)
      {
        modifiedItem->appendValue(m.component());
      }
    }
    else if (optype == "Remove")
    {
      smtk::attribute::ComponentItem::Ptr expungedItem = result->findComponent("expunged");
      for (auto& e : remGroups)
      {
        expungedItem->appendValue(e.component());
      }
    }
  }
  return result;
}

const char* EntityGroupOperation::xmlDescription() const
{
  return EntityGroupOperation_xml;
}

} //namespace model
} // namespace smtk

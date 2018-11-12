//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/EditAssembly.h"

#include "smtk/session/rgg/Session.h"
#include "smtk/session/rgg/operators/CreateAssembly.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/model/operators/CreateInstances.h"

#include "smtk/session/rgg/EditAssembly_xml.h"

#include <limits>
#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace rgg
{

EditAssembly::Result EditAssembly::operateInternal()
{
  Result result = this->createResult(smtk::operation::Operation::Outcome::FAILED);

  EntityRefArray entities = this->parameters()->associatedModelEntities<EntityRefArray>();
  if (entities.empty() || !entities[0].isGroup())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non group type entity");
    return result;
  }

  smtk::model::Group assembly = entities[0].as<smtk::model::Group>();
  CreateAssembly::populateAssembly(this, assembly);
  // Remove and add instances
  smtk::attribute::ModelEntityItemPtr dI =
    this->parameters()->findModelEntity("instance to be deleted");
  for (auto it = dI->begin(); it != dI->end(); it++)
  {
    auto entity = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
    assembly.removeEntity(smtk::model::EntityRef(entity->modelResource(), entity->id()));
  }

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ModelEntityItemPtr aI =
    this->parameters()->findModelEntity("instance to be added");
  for (auto it = aI->begin(); it != aI->end(); it++)
  {
    auto entity = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
    auto eRef = smtk::model::EntityRef(entity->modelResource(), entity->id());
    // Instances would inheret its related prototype's color
    smtk::model::Instance instance = eRef.as<smtk::model::Instance>();
    instance.setColor(instance.prototype().color());
    std::string iName = "instance_Of_" + instance.prototype().name();
    instance.setName(iName);
    result->findComponent("modified")->appendValue(instance.component());

    assembly.addEntity(eRef);
  }

  result->findComponent("modified")->appendValue(assembly.component());
  return result;
}

const char* EditAssembly::xmlDescription() const
{
  return EditAssembly_xml;
}
} // namespace rgg
} //namespace session
} // namespace smtk

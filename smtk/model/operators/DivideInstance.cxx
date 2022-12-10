//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/DivideInstance.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Instance.txx"
#include "smtk/model/Resource.h"
#include "smtk/model/Resource.txx"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/operators/DivideInstance_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

bool DivideInstance::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // Check that the associated instance is cloned or has cloned children
  // that we can use to divide its placements.
  Instance parent = this->parentOfClones(this->parameters()->associations());
  return parent.isValid();
}

DivideInstance::Result DivideInstance::operateInternal()
{
  auto associations = this->parameters()->associations();
  Instance instance = this->parentOfClones(associations);
  smtk::model::ResourcePtr resource = instance.resource();
  if (!resource || !instance.isValid())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto created = result->findComponent("created");
  auto modified = result->findComponent("modified");
  auto expunged = result->findComponent("expunged");

  // Divide instance.
  std::set<smtk::model::Instance> children;
  auto divided = instance.divide(/* merge */ false, &children);
  if (divided.find(instance) == divided.end())
  {
    // The output instance is not the input. So it is
    // safe to delete the input.
    children.insert(children.end(), instance);
  }
  for (const auto& entry : children)
  {
    expunged->appendValue(entry.entityRecord());
  }
  smtk::model::EntityRefArray delExpunged;
  smtk::model::EntityRefArray delModified;
  resource->deleteEntities(children, delModified, delExpunged, m_debugLevel > 0);
  for (const auto& entry : divided)
  {
    if (entry == instance)
    {
      continue;
    }
    created->appendValue(entry.entityRecord());
  }
  for (const auto& tmp : delExpunged)
  {
    expunged->appendValue(tmp.entityRecord());
  }
  for (const auto& tmp : delModified)
  {
    modified->appendValue(tmp.entityRecord());
  }

  return result;
}

const char* DivideInstance::xmlDescription() const
{
  return DivideInstance_xml;
}

smtk::model::Instance DivideInstance::parentOfClones(
  const smtk::attribute::ReferenceItemPtr& item) const
{
  auto instances = item->as<Instances>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Instance(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  if (instances.empty())
  {
    smtkErrorMacro(this->log(), "No instances were provided.");
    return smtk::model::Instance();
  }

  Instance result;
  for (auto instance : instances)
  {
    Instance parent;
    for (; (parent = instance.memberOf()).isValid();)
    {
      if (parent.isInstance())
      {
        instance = parent;
      }
      else
      {
        break;
      }
    }
    if (!result.isValid())
    {
      result = instance;
    }
    else if (result != instance)
    {
      smtkErrorMacro(
        this->log(),
        "The given instance has clones with different parent "
        "instances ("
          << result.name() << ", " << instance.name() << ").");
      return smtk::model::Instance();
    }
  }
  smtk::model::ResourcePtr resource = result.resource();
  if (!resource || !result.isValid())
  {
    smtkErrorMacro(this->log(), "The given instance is invalid.");
    return smtk::model::Instance();
  }

  return result;
}

} //namespace model
} // namespace smtk

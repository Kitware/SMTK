//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/MergeInstances.h"

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

#include "smtk/model/operators/MergeInstances_xml.h"

#include <algorithm>

using namespace smtk::model;

namespace smtk
{
namespace model
{

bool MergeInstances::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }
  auto associations = this->parameters()->associations();
  auto instances = associations->as<Instances>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Instance(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  smtk::model::EntityRef proto;
  if (instances.empty())
  {
    return false;
  }
  proto = instances.begin()->prototype();
  if (!proto.isValid())
  {
    return false;
  }
  return std::all_of(instances.begin(), instances.end(), [&proto](const Instance& instance) {
    return instance.prototype() == proto;
  });
}

MergeInstances::Result MergeInstances::operateInternal()
{
  auto associations = this->parameters()->associations();
  auto instances = associations->as<Instances>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Instance(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  if (instances.empty())
  {
    smtkErrorMacro(this->log(), "No instance specified.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  smtk::model::ResourcePtr resource = instances[0].resource();
  if (!resource)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Instance merged = Instance::merge(instances);
  if (!merged.isValid())
  {
    smtkErrorMacro(this->log(), "Instances could not be merged.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto created = result->findComponent("created");
  auto modified = result->findComponent("modified");
  auto expunged = result->findComponent("expunged");

  created->appendValue(merged.entityRecord());
  for (const auto& instance : instances)
  {
    expunged->appendValue(instance.entityRecord());
  }
  smtk::model::EntityRefArray delExpunged;
  smtk::model::EntityRefArray delModified;
  resource->deleteEntities(instances, delModified, delExpunged, m_debugLevel > 0);
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

const char* MergeInstances::xmlDescription() const
{
  return MergeInstances_xml;
}

} //namespace model
} // namespace smtk

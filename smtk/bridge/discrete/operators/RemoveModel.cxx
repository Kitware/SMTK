//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/discrete/operators/RemoveModel.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "RemoveModel_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace discrete
{

bool RemoveModel::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
    return false;
  auto associations = this->parameters()->associations();
  std::size_t numModels = associations->numberOfValues();
  return numModels > 0;
}

RemoveModel::Result RemoveModel::operateInternal()
{
  // ableToOperate should have verified that model(s) are set
  bool success = true;
  smtk::resource::ComponentArray expunged;
  auto associations = this->parameters()->associations();
  auto remModels = associations->as<Models>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Model(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  for (Models::iterator it = remModels.begin(); it != remModels.end(); ++it)
  {
    smtk::bridge::discrete::Resource::Ptr resource =
      std::static_pointer_cast<smtk::bridge::discrete::Resource>(it->component()->resource());
    expunged.push_back(it->component());
    success = resource->discreteSession()->removeModelEntity(*it);
    if (!success)
      break;
  }

  Result result = this->createResult(success ? smtk::operation::Operation::Outcome::SUCCEEDED
                                             : smtk::operation::Operation::Outcome::FAILED);

  if (success)
  {
    smtk::attribute::ComponentItem::Ptr expungedItem = result->findComponent("expunged");
    for (auto e : expunged)
    {
      expungedItem->appendValue(e);
    }
  }
  return result;
}

const char* RemoveModel::xmlDescription() const
{
  return RemoveModel_xml;
}

} // namespace discrete
} // namespace bridge
} // namespace smtk

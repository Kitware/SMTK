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
#include "smtk/attribute/ModelEntityItem.h"

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
  Models remModels(associations->begin(), associations->end());
  for (Models::iterator it = remModels.begin(); it != remModels.end(); ++it)
  {
    smtk::bridge::discrete::Resource::Ptr resource =
      std::static_pointer_cast<smtk::bridge::discrete::Resource>(it->component()->resource());
    expunged.push_back(it->component());
    success = resource->discreteSession()->removeModelEntity(*it);
    if (!success)
      break;
  }

  OperatorResult result = this->createResult(
    success ? smtk::operation::NewOp::Outcome::SUCCEEDED : smtk::operation::NewOp::Outcome::FAILED);

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

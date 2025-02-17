//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/oscillator/operators/CreateModel.h"

#include "smtk/session/oscillator/Resource.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Model.h"

#include "smtk/session/oscillator/operators/CreateModel_xml.h"
using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace oscillator
{

CreateModel::Result CreateModel::operateInternal()
{
  Result result;

  // There are two possible create modes
  // 1. Create a model within an existing resource

  smtk::session::oscillator::Resource::Ptr resource = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  auto existingResourceItem = this->parameters()->associations();
  if (existingResourceItem && existingResourceItem->numberOfValues() > 0)
  {
    resource =
      std::static_pointer_cast<smtk::session::oscillator::Resource>(existingResourceItem->value(0));
  }

  if (!resource)
  {
    // If no existing resource is provided, then we create a new resource.
    resource = smtk::session::oscillator::Resource::create();
  }

  smtk::model::Model model = resource->addModel(/* par. dim. */ 3, /* emb. dim. */ 3);
  model.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);

  auto nameItem = this->parameters()->findString("name");
  std::string modelName;
  if (nameItem && nameItem->isEnabled())
  {
    model.setName(nameItem->value(0));
  }
  // Do not allow an empty name from the user:
  if (model.name().empty())
  {
    model.assignDefaultName();
  }

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->appendValue(model.component());
  }

  if (!result)
  {
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  return result;
}

const char* CreateModel::xmlDescription() const
{
  return CreateModel_xml;
}
} // namespace oscillator
} //namespace session
} // namespace smtk

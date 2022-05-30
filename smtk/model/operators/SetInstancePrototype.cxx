//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/SetInstancePrototype.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/operators/SetInstancePrototype_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

SetInstancePrototype::Result SetInstancePrototype::operateInternal()
{
  auto associations = this->parameters()->associations();
  auto entities = associations->as<Instances>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Instance(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No instance specified.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Instance instance = entities[0];

  auto prototypeItem = this->parameters()->findComponent("prototype");
  auto prototypeRec = prototypeItem->valueAs<smtk::model::Entity>(0);
  EntityRef prototype(prototypeRec);
  if (!prototype.isValid())
  {
    smtkErrorMacro(this->log(), "No prototype specified.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  EntityRef oldProto = instance.prototype();
  bool ok = instance.setPrototype(prototype);

  Result result = this->createResult(
    ok ? smtk::operation::Operation::Outcome::SUCCEEDED
       : smtk::operation::Operation::Outcome::FAILED);

  // Mark entities as modified. Note that the original and
  // the new prototype should be included so descriptive
  // phrases noting their instances can be updated.
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
  modified->appendValue(instance.component());
  modified->appendValue(prototype.component());
  modified->appendValue(oldProto.component());

  return result;
}

const char* SetInstancePrototype::xmlDescription() const
{
  return SetInstancePrototype_xml;
}

} //namespace model
} // namespace smtk

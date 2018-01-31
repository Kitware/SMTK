//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/CreateAssembly.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/attribute/StringItem.h"
#include "smtk/io/Logger.h"

#include "smtk/bridge/rgg/CreateAssembly_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult CreateAssembly::operateInternal()
{
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  if (!sess)
  {
    return result;
  }
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  // Since we are in creating mode, it must be a model
  if (entities.empty() || !entities[0].isModel())
  {
    smtkErrorMacro(this->log(), "An invalid model is provided for create pin op");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  EntityRef parent = entities[0];
  smtk::model::AuxiliaryGeometry auxGeom;
  auxGeom = parent.manager()->addAuxiliaryGeometry(parent.as<smtk::model::Model>(), 3);
  auxGeom.setStringProperty("rggType", SMTK_BRIDGE_RGG_ASSEMBLY);

  smtk::attribute::StringItemPtr nameItem = this->findString("name");
  std::string pinName;
  if (nameItem != nullptr && !nameItem->value(0).empty())
  {
    pinName = nameItem->value(0);
    auxGeom.setName(nameItem->value(0));
  }

  smtk::attribute::StringItemPtr labelItem = this->findString("label");
  if (labelItem != nullptr && !labelItem->value(0).empty())
  {
    auxGeom.setStringProperty(labelItem->name(), labelItem->value(0));
  }

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  this->addEntityToResult(result, auxGeom, CREATED);
  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::CreateAssembly,
  rgg_create_assembly, "create assembly", CreateAssembly_xml, smtk::bridge::rgg::Session);

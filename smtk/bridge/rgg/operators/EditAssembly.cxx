//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/EditAssembly.h"

#include "smtk/bridge/rgg/Session.h"

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
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/operators/CreateInstances.h"

#include "smtk/bridge/rgg/EditAssembly_xml.h"

#include <limits>
#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult EditAssembly::operateInternal()
{
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_FAILED);

  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  if (!sess || !sess->manager())
  {
    return result;
  }
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  if (entities.empty() || !entities[0].isGroup())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non group type entity");
    return result;
  }

  smtk::model::Group assembly = entities[0].as<smtk::model::Group>();
  CreateAssembly::populateAssembly(this, assembly);
  // Remove and add instances
  smtk::attribute::ModelEntityItemPtr dI = this->findModelEntity("instance to be deleted");
  for (auto it = dI->begin(); it != dI->end(); it++)
  {
    assembly.removeEntity(*it);
  }

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  smtk::attribute::ModelEntityItemPtr aI = this->findModelEntity("instance to be added");
  for (auto it = aI->begin(); it != aI->end(); it++)
  {
    // Instances would inheret its related prototype's color
    smtk::model::Instance instance = it->as<smtk::model::Instance>();
    instance.setColor(instance.prototype().color());
    std::string iName = "instance_Of_" + instance.prototype().name();
    instance.setName(iName);
    this->addEntityToResult(result, instance, MODIFIED);

    assembly.addEntity(*it);
  }

  this->addEntityToResult(result, assembly, MODIFIED);
  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::EditAssembly,
  rgg_edit_assembly, "edit assembly", EditAssembly_xml, smtk::bridge::rgg::Session);

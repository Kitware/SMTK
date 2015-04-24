//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/RemoveModel.h"

#include "smtk/bridge/cgm/Session.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
  namespace cgm {

bool RemoveModel::ableToOperate()
{
  if(!this->ensureSpecification())
    return false;
  std::size_t numModels = this->associatedEntitiesAs<Models>().size();
  return numModels > 0;
}

smtk::model::OperatorResult RemoveModel::operateInternal()
{
  // ableToOperate should have verified that model(s) are set
  bool success = true;
  EntityRefArray expunged;
  Models remModels = this->associatedEntitiesAs<Models>();
  for(Models::iterator it = remModels.begin();
      it != remModels.end(); ++it)
    {
    success = this->cgmSession()->removeModelEntity(*it);
    if(!success)
      break;
    expunged.push_back(*it);
    }

  OperatorResult result =
    this->createResult(
      success ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (success)
    result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());
  return result;
}

    }
  } //namespace model
} // namespace smtk

#include "RemoveModel_xml.h"

smtkImplementsModelOperator(
  SMTKCGMSESSION_EXPORT,
  smtk::bridge::cgm::RemoveModel,
  cgm_remove_model,
  "remove model",
  RemoveModel_xml,
  smtk::bridge::cgm::Session);

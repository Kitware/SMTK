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
#include "smtk/bridge/cgm/RemoveModel_xml.h"

#include "Body.hpp"
#include "GeometryQueryTool.hpp"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace cgm
{

bool RemoveModel::ableToOperate()
{
  if (!this->ensureSpecification())
    return false;
  std::size_t numModels = this->associatedEntitiesAs<Models>().size();
  return numModels > 0;
}

smtk::operation::OperationResult RemoveModel::operateInternal()
{
  GeometryQueryTool* gqt = GeometryQueryTool::instance();
  if (!gqt)
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);

  // ableToOperate should have verified that model(s) are set
  DLIList<Body*> bodies;
  EntityRefArray expunged;
  if (!this->cgmEntities(this->associatedEntitiesAs<Models>(), bodies, false, expunged))
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);

  // This does not return an error code; assume success.
  gqt->delete_Body(bodies);

  OperationResult result = this->createResult(smtk::operation::Operation::OPERATION_SUCCEEDED);

  result->findComponent("expunged")->setValues(expunged.begin(), expunged.end());
  return result;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperation(SMTKCGMSESSION_EXPORT, smtk::bridge::cgm::RemoveModel,
  cgm_remove_model, "remove model", RemoveModel_xml, smtk::bridge::cgm::Session);

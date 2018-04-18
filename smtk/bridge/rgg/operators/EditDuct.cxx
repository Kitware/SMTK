//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/EditDuct.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/bridge/rgg/operators/CreateDuct.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/EditDuct_xml.h"

#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult EditDuct::operateInternal()
{

  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  if (!sess)
  {
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  if (entities.empty() || !entities[0].isAuxiliaryGeometry())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non auxiliary geometry entity");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  smtk::model::EntityRefArray expunged, modified, tobeDeleted;

  smtk::model::AuxiliaryGeometry auxGeom = entities[0].as<AuxiliaryGeometry>();
  // Remove all current child auxiliary geometries first
  EntityRefArray children = auxGeom.embeddedEntities<EntityRefArray>();
  auxGeom.setIntegerProperty("previous children size", static_cast<int>(children.size()));
  tobeDeleted.insert(tobeDeleted.end(), children.begin(), children.end());

  if (this->manager())
  {
    this->session()->manager()->deleteEntities(
      tobeDeleted, modified, expunged, this->m_debugLevel > 0);
  }

  // A list contains all segments and layers of the duct
  std::vector<EntityRef> subAuxGeoms;

  CreateDuct::populateDuct(dynamic_cast<smtk::model::Operator*>(this), auxGeom, subAuxGeoms);

  this->addEntityToResult(result, auxGeom, MODIFIED);
  this->addEntitiesToResult(result, subAuxGeoms, CREATED);
  this->addEntitiesToResult(result, modified, MODIFIED);
  this->addEntitiesToResult(result, expunged, EXPUNGED);
  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::EditDuct, rgg_edit_duct,
  "edit duct", EditDuct_xml, smtk::bridge::rgg::Session);

//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/DemoteVertex.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/DemoteVertex_xml.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

smtk::model::OperatorResult DemoteVertex::operateInternal()
{
  smtk::bridge::polygon::SessionPtr sess = this->polygonSession();
  smtk::model::Manager::Ptr mgr;
  if (!sess)
    return this->createResult(smtk::model::OPERATION_FAILED);

  mgr = sess->manager();

  smtk::attribute::ModelEntityItem::Ptr vertItem = this->specification()->associations();
  smtk::model::Vertex vertexToDemote(vertItem->value(0));
  if (!vertexToDemote.isValid())
  {
    smtkErrorMacro(this->log(), "The input vertex (" << vertexToDemote.entity() << ") is invalid.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  internal::vertex::Ptr storage = this->findStorage<internal::vertex>(vertexToDemote.entity());
  internal::pmodel* mod = storage->parentAs<internal::pmodel>();
  if (!storage || !mod)
  {
    smtkErrorMacro(this->log(), "The input vertex has no storage or no parent model set.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  smtk::model::EntityRefs created;
  smtk::model::EntityRefs modified;
  smtk::model::EntityRefs expunged;
  bool ok = mod->demoteModelVertex(mgr, storage, created, modified, expunged);
  smtk::model::OperatorResult opResult;
  if (ok)
  {
    opResult = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(
      opResult, smtk::model::EntityRefArray(created.begin(), created.end()), CREATED);
    this->addEntitiesToResult(
      opResult, smtk::model::EntityRefArray(modified.begin(), modified.end()), MODIFIED);
    this->addEntitiesToResult(
      opResult, smtk::model::EntityRefArray(expunged.begin(), expunged.end()), EXPUNGED);
  }
  else
  {
    opResult = this->createResult(smtk::model::OPERATION_FAILED);
  }

  return opResult;
}

} // namespace polygon
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKPOLYGONSESSION_EXPORT, smtk::bridge::polygon::DemoteVertex,
  polygon_demote_vertex, "demote vertex", DemoteVertex_xml, smtk::bridge::polygon::Session);

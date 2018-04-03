//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/SplitEdge.h"

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/SplitEdge_xml.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

SplitEdge::Result SplitEdge::operateInternal()
{
  smtk::attribute::IntItem::Ptr pointIdItem = this->parameters()->findInt("point id");
  smtk::attribute::DoubleItem::Ptr pointItem = this->parameters()->findDouble("point");
  auto edgeItem = this->parameters()->associations();
  smtk::model::Edge edgeToSplit(edgeItem->valueAs<smtk::model::Entity>());
  if (!edgeToSplit.isValid())
  {
    smtkErrorMacro(this->log(), "The input edge (" << edgeToSplit.entity() << ") is invalid.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::bridge::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::polygon::Resource>(edgeToSplit.component()->resource());
  if (!resource)
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  internal::edge::Ptr storage = resource->findStorage<internal::edge>(edgeToSplit.entity());
  internal::pmodel* mod = storage->parentAs<internal::pmodel>();
  if (!storage || !mod)
  {
    smtkErrorMacro(this->log(), "The input edge has no storage or no parent model set.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::vector<double> point;
  smtk::model::EntityRefArray created;
  bool ok;
  if (pointIdItem && pointIdItem->value(0) >= 0 &&
    pointIdItem->value(0) < static_cast<int>(storage->pointsSize()))
  {
    ok = mod->splitModelEdgeAtIndex(
      resource, edgeToSplit.entity(), pointIdItem->value(0), created, m_debugLevel);
  }
  else
  {
    point = std::vector<double>(pointItem->begin(), pointItem->end());
    ok = mod->splitModelEdgeAtPoint(resource, edgeToSplit.entity(), point, created, m_debugLevel);
  }
  Result opResult;
  if (ok)
  {
    opResult = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

    smtk::attribute::ComponentItem::Ptr createdItem = opResult->findComponent("created");
    for (auto& c : created)
    {
      createdItem->appendValue(c.component());
    }

    smtk::attribute::ComponentItem::Ptr expungedItem = opResult->findComponent("expunged");
    expungedItem->appendValue(edgeToSplit.component());
  }
  else
  {
    smtkErrorMacro(this->log(), "Failed to split edge.");
    opResult = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return opResult;
}

const char* SplitEdge::xmlDescription() const
{
  return SplitEdge_xml;
}

} // namespace polygon
} //namespace bridge
} // namespace smtk

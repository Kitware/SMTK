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

#include "smtk/bridge/polygon/SplitEdge_xml.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

smtk::model::OperatorResult SplitEdge::operateInternal()
{
  smtk::bridge::polygon::Session* sess = this->polygonSession();
  smtk::model::Manager::Ptr mgr;
  if (!sess)
    return this->createResult(smtk::model::OPERATION_FAILED);

  mgr = sess->manager();

  smtk::attribute::DoubleItem::Ptr pointItem = this->findDouble("point");
  smtk::attribute::ModelEntityItem::Ptr edgeItem = this->specification()->associations();
  smtk::model::Edge edgeToSplit(edgeItem->value(0));
  if (!edgeToSplit.isValid())
    {
    smtkErrorMacro(this->log(),
      "The input edge (" << edgeToSplit.entity() << ") is invalid.");
      return this->createResult(smtk::model::OPERATION_FAILED);
    }

  internal::edge::Ptr storage =
    this->findStorage<internal::edge>(
      edgeToSplit.entity());
  internal::pmodel* mod = storage->parentAs<internal::pmodel>();
  if (!storage || !mod)
    {
    smtkErrorMacro(this->log(),
      "The input edge has no storage or no parent model set.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  std::vector<double> point(pointItem->begin(), pointItem->end());
  smtk::model::EntityRefs created;
  smtk::model::EntityRefs modified;
  bool ok = mod->splitModelEdgeAtPoint(mgr, edgeToSplit.entity(), point, created, modified);
  smtk::model::OperatorResult opResult;
  if (ok)
    {
    smtk::bridge::polygon::internal::Point pt = mod->projectPoint(point.begin(), point.end());
    smtk::model::EntityRefArray created;
    smtk::model::Vertex v = mod->vertexAtPoint(mgr, pt);
    smtk::model::Edges edges = v.edges();
    created.insert(created.end(), edges.begin(), edges.end());
    // no need to add the new vertex since it will be under new edges
    // created.push_back(v);

    opResult = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(opResult, created, CREATED);
    this->addEntitiesToResult(opResult, smtk::model::EntityRefArray(modified.begin(), modified.end()), MODIFIED);
    this->addEntityToResult(opResult, edgeToSplit, EXPUNGED);
    }
  else
    {
    smtkErrorMacro(this->log(), "Failed to split edge.");
    opResult = this->createResult(smtk::model::OPERATION_FAILED);
    }

  return opResult;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::SplitEdge,
  polygon_split_edge,
  "split edge",
  SplitEdge_xml,
  smtk::bridge::polygon::Session);

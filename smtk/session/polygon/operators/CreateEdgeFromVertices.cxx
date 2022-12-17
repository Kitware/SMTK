//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/CreateEdgeFromVertices.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/session/polygon/operators/CreateEdgeFromVertices_xml.h"

namespace smtk
{
namespace session
{
namespace polygon
{

typedef std::vector<std::pair<size_t, internal::Segment>> SegmentSplitsT;

CreateEdgeFromVertices::Result CreateEdgeFromVertices::operateInternal()
{
  auto modelItem = this->parameters()->associations();
  auto ment = modelItem->valueAs<smtk::model::Entity>();
  smtk::model::Model parentModel = smtk::model::EntityRef(ment).owningModel();

  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(parentModel.component()->resource());

  if (!resource)
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  if (!(parentModel.isValid() && (modelItem->numberOfValues() == 2)))
  {
    smtkErrorMacro(
      this->log(),
      "A model (or vertices with a valid parent model) must be associated with the operator.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  smtk::model::EntityPtr evert[2] = { modelItem->valueAs<smtk::model::Entity>(0),
                                      modelItem->valueAs<smtk::model::Entity>(1) };
  if (!(evert[0]->isVertex() && evert[1]->isVertex()))
  {
    smtkErrorMacro(
      this->log(),
      "When constructing an edge from vertices,"
      " all associated model entities must be vertices"
      " and there must be 2 vertices");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  internal::pmodel::Ptr storage = resource->findStorage<internal::pmodel>(parentModel.entity());
  bool ok = true;
  internal::vertex::Ptr verts[2];
  smtk::model::VertexSet freeVerts = parentModel.cellsAs<smtk::model::VertexSet>();
  // Lets check to make sure the vertices are valid
  for (int i = 0; i < 2; i++)
  {
    verts[i] = resource->findStorage<internal::vertex>(modelItem->value(i)->id());
    if (!verts[i])
    {
      smtkErrorMacro(
        this->log(),
        "When constructing an edge from vertices, Vertex " << i << " does not apprear to be valid");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }

  // If either of these verices are "owned" by the model - meaning it has no bordants using it
  // its ownership will change when the edge is created and the model will be considered modified
  smtk::model::Edges created;
  smtk::model::Vertex vert0(resource, verts[0]->id());
  smtk::model::Vertex vert1(resource, verts[1]->id());

  smtk::model::Edge edge = storage->createModelEdgeFromVertices(resource, verts[0], verts[1]);
  if (edge.isValid())
  {
    created.push_back(edge);
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
    operation::MarkGeometry(resource).markModified(createdItem);

    smtk::attribute::ComponentItem::Ptr modified = opResult->findComponent("modified");
    modified->appendValue(parentModel.component());
  }
  else
  {
    opResult = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return opResult;
}

const char* CreateEdgeFromVertices::xmlDescription() const
{
  return CreateEdgeFromVertices_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk

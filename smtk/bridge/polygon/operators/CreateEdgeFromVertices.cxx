//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/CreateEdgeFromVertices.h"

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

#include "smtk/bridge/polygon/CreateEdgeFromVertices_xml.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

typedef std::vector<std::pair<size_t, internal::Segment> > SegmentSplitsT;

/*
template<typename T>
void printSegment(internal::pmodel::Ptr storage, const std::string& msg, T& seg)
{
  std::vector<double> lo(3);
  std::vector<double> hi(3);
  storage->liftPoint(seg.low(), lo.begin());
  storage->liftPoint(seg.high(), hi.begin());
  std::cout
    << msg
    << "    " << lo[0] << " " << lo[1] << " " << lo[2]
    << " -> " << hi[0] << " " << hi[1] << " " << hi[2]
    << "\n";
}
*/

smtk::model::OperatorResult CreateEdgeFromVertices::operateInternal()
{
  smtk::bridge::polygon::Session* sess = this->polygonSession();
  smtk::model::Manager::Ptr mgr;
  if (!sess)
    return this->createResult(smtk::model::OPERATION_FAILED);

  mgr = sess->manager();
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();
  smtk::model::Model parentModel(modelItem->value(0));
  if (!parentModel.isValid())
  {
    parentModel = modelItem->value(0).owningModel();
    if (!(parentModel.isValid() && (modelItem->numberOfValues() == 2)))
    {
      smtkErrorMacro(this->log(),
        "A model (or vertices with a valid parent model) must be associated with the operator.");
      return this->createResult(smtk::model::OPERATION_FAILED);
    }
  }
  if (!(modelItem->value(0).isVertex() && modelItem->value(1).isVertex()))
  {
    smtkErrorMacro(this->log(), "When constructing an edge from vertices,"
                                " all associated model entities must be vertices"
                                " and there must be 2 vertices");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  internal::pmodel::Ptr storage = this->findStorage<internal::pmodel>(parentModel.entity());
  bool ok = true;
  internal::vertex::Ptr verts[2];
  smtk::model::VertexSet freeVerts = parentModel.cellsAs<smtk::model::VertexSet>();
  // Lets check to make sure the vertices are valid
  for (int i = 0; i < 2; i++)
  {
    verts[i] = this->findStorage<internal::vertex>(modelItem->value(i).entity());
    if (!verts[i])
    {
      smtkErrorMacro(this->log(), "When constructing an edge from vertices, Vertex "
          << i << " does not apprear to be valid");
      return this->createResult(smtk::model::OPERATION_FAILED);
    }
  }

  // If either of these verices are "owned" by the model - meaning it has no bordants using it
  // its ownership will change when the edge is created and the model will be considered modified
  smtk::model::EntityRefArray modified;
  smtk::model::Edges created;
  smtk::model::Vertex vert0(mgr, verts[0]->id());
  smtk::model::Vertex vert1(mgr, verts[1]->id());

  if (parentModel.isEmbedded(vert0) || parentModel.isEmbedded(vert1))
  {
    modified.push_back(parentModel);
  }

  smtk::model::Edge edge = storage->createModelEdgeFromVertices(mgr, verts[0], verts[1]);
  if (edge.isValid())
  {
    created.push_back(edge);
  }

  smtk::model::OperatorResult opResult;
  if (ok)
  {
    opResult = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(opResult, created, CREATED);
    this->addEntitiesToResult(opResult, modified, MODIFIED);
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

smtkImplementsModelOperator(SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::CreateEdgeFromVertices, polygon_create_edge_from_vertices,
  "create edge from vertices", CreateEdgeFromVertices_xml, smtk::bridge::polygon::Session);

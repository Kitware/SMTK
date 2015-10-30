//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/CreateVertices.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateVertices_xml.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

smtk::model::OperatorResult CreateVertices::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr pointsItem = this->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("coordinates");
  int numCoordsPerPt = coordinatesItem->value(0);
  numCoordsPerPt =
    (numCoordsPerPt < 2 ? 2 :
     (numCoordsPerPt > 3 ? 3 :
      numCoordsPerPt));

  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();

  smtk::bridge::polygon::Session* sess = this->polygonSession();
  smtk::model::OperatorResult result;
  if (sess)
    {
    smtk::model::Manager::Ptr mgr = sess->manager();
    smtk::model::Model model = modelItem->value(0);
    internal::pmodel::Ptr storage =
      sess->findStorage<internal::pmodel>(
        model.entity());
    std::vector<double> pcoords(pointsItem->begin(), pointsItem->end());
    smtk::model::Vertices verts =
      storage->findOrAddModelVertices(mgr, pcoords, numCoordsPerPt);
    result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(result, verts, CREATED);
    for (smtk::model::Vertices::const_iterator it = verts.begin(); it != verts.end(); ++it)
      { // Add raw relationships from model to/from vertex:
      model.addCell(*it);
      }
    }
  if (!result)
    {
    result = this->createResult(smtk::model::OPERATION_FAILED);
    }

  return result;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::CreateVertices,
  polygon_create_vertices,
  "create vertices",
  CreateVertices_xml,
  smtk::bridge::polygon::Session);

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
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateVertices_xml.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

smtk::model::OperatorResult CreateVertices::operateInternal()
{
  smtk::attribute::GroupItem::Ptr pointsInfo;
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("point dimension");
  int numCoordsPerPt = coordinatesItem->value();
  if (numCoordsPerPt == 2)
    {
    pointsInfo = this->findGroup("2d points");
    }
  else
    {
    pointsInfo = this->findGroup("3d points");
    }
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();

  smtk::bridge::polygon::Session* sess = this->polygonSession();
  smtk::model::OperatorResult result;
  if (sess)
    {
    smtk::model::Manager::Ptr mgr = sess->manager();
    smtk::model::Model model = modelItem->value(0);
    internal::pmodel::Ptr storage =
      this->findStorage<internal::pmodel>(
        model.entity());
    std::vector<double> pcoords;
    int npnts = static_cast<int>(pointsInfo->numberOfGroups());
    pcoords.reserve(npnts * numCoordsPerPt);

    // Save the points into the vector to be processed by create vertex method
    for (int i = 0; i < npnts; i++)
      {
      for (int j = 0; j < numCoordsPerPt; j++)
        {
        pcoords.push_back(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(pointsInfo->item(i,0))->value(j));
        }
      }

    smtk::model::Vertices verts =
      storage->findOrAddModelVertices(mgr, pcoords, numCoordsPerPt);
    result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(result, verts, CREATED);
    for (smtk::model::Vertices::const_iterator it = verts.begin(); it != verts.end(); ++it)
      { // Add raw relationships from model to/from vertex:
      model.addCell(*it);
      }
    this->addEntityToResult(result, model, MODIFIED);
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

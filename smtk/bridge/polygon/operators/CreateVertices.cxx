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

#include "smtk/bridge/polygon/Resource.h"
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

namespace smtk
{
namespace bridge
{
namespace polygon
{

CreateVertices::Result CreateVertices::operateInternal()
{
  smtk::attribute::GroupItem::Ptr pointsInfo;
  smtk::attribute::IntItem::Ptr coordinatesItem = this->parameters()->findInt("point dimension");
  int numCoordsPerPt = coordinatesItem->value();
  if (numCoordsPerPt == 2)
  {
    pointsInfo = this->parameters()->findGroup("2d points");
  }
  else
  {
    pointsInfo = this->parameters()->findGroup("3d points");
  }
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->parameters()->associations();

  smtk::model::Model model = modelItem->value(0);

  smtk::bridge::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::polygon::Resource>(model.component()->resource());

  Result result;
  if (resource)
  {
    smtk::model::Manager::Ptr mgr = std::static_pointer_cast<smtk::model::Manager>(resource);
    internal::pmodel::Ptr storage = resource->findStorage<internal::pmodel>(model.entity());
    std::vector<double> pcoords;
    int npnts = static_cast<int>(pointsInfo->numberOfGroups());
    pcoords.reserve(npnts * numCoordsPerPt);

    // Save the points into the vector to be processed by create vertex method
    for (int i = 0; i < npnts; i++)
    {
      for (int j = 0; j < numCoordsPerPt; j++)
      {
        pcoords.push_back(
          smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(pointsInfo->item(i, 0))
            ->value(j));
      }
    }

    smtk::model::Vertices verts = storage->findOrAddModelVertices(mgr, pcoords, numCoordsPerPt);
    result = this->createResult(smtk::operation::NewOp::Outcome::SUCCEEDED);

    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");

    for (smtk::model::Vertices::const_iterator it = verts.begin(); it != verts.end(); ++it)
    {
      created->appendValue(it->component());
      model.addCell(*it);
    }

    smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
    modified->setValue(model.component());
  }
  if (!result)
  {
    result = this->createResult(smtk::operation::NewOp::Outcome::FAILED);
  }

  return result;
}

const char* CreateVertices::xmlDescription() const
{
  return CreateVertices_xml;
}

} // namespace polygon
} //namespace bridge
} // namespace smtk

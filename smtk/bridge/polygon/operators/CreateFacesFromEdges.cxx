//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/CreateFacesFromEdges.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/ActiveFragmentTree.h"
#include "smtk/bridge/polygon/internal/Config.h"
#include "smtk/bridge/polygon/internal/Edge.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Neighborhood.h"
#include "smtk/bridge/polygon/internal/Region.h"
#include "smtk/bridge/polygon/internal/SweepEvent.h"
#include "smtk/bridge/polygon/internal/Util.h"

#include "smtk/bridge/polygon/Operator.txx"
#include "smtk/bridge/polygon/internal/Model.txx"
#include "smtk/bridge/polygon/internal/Neighborhood.txx"

#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/common/UnionFind.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateFacesFromEdges_xml.h"

#include <deque>
#include <map>
#include <set>
#include <vector>

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

namespace smtk {
  namespace bridge {
    namespace polygon {

bool CreateFacesFromEdges::populateEdgeMap()
{
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();
  smtk::model::Model model;

  internal::pmodel::Ptr storage; // Look up from session = internal::pmodel::create();

  // First, collect  edges to process:
  for (int i = 0; i < static_cast<int>(modelItem->numberOfValues()); ++i)
    {
    smtk::model::Edge edgeIn(modelItem->value(i));
    if (edgeIn.isValid())
      {
      if (model.isValid())
        {
        if (model != edgeIn.owningModel())
          {
          smtkErrorMacro(this->log(),
                         "Edges from different models (" << model.name() <<
                         " and " << edgeIn.owningModel().name() << ") selected.");
          this->m_result = this->createResult(smtk::model::OPERATION_FAILED);
          return false;
          }
        }
      else
        {
        model = edgeIn.owningModel();
        }
      this->m_edgeMap[edgeIn] = 0;
      }
    }
  if (this->m_edgeMap.empty() || !model.isValid())
    {
    smtkErrorMacro(this->log(), "No edges selected or invalid model specified.");
    this->m_result = this->createResult(smtk::model::OPERATION_FAILED);
    return false;
    }
  this->m_model = model;
  return true;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::CreateFacesFromEdges,
  polygon_create_faces_from_edges,
  "create faces from edges",
  CreateFacesFromEdges_xml,
  smtk::bridge::polygon::Session);

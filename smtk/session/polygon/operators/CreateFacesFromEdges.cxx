//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/CreateFacesFromEdges.h"

#include "smtk/session/polygon/Session.h"
#include "smtk/session/polygon/internal/ActiveFragmentTree.h"
#include "smtk/session/polygon/internal/Config.h"
#include "smtk/session/polygon/internal/Edge.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Neighborhood.h"
#include "smtk/session/polygon/internal/Region.h"
#include "smtk/session/polygon/internal/SweepEvent.h"
#include "smtk/session/polygon/internal/Util.h"

#include "smtk/session/polygon/Operation.txx"
#include "smtk/session/polygon/internal/Model.txx"
#include "smtk/session/polygon/internal/Neighborhood.txx"

#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/common/UnionFind.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/polygon/operators/CreateFacesFromEdges_xml.h"

#include <deque>
#include <map>
#include <set>
#include <vector>

using namespace boost::polygon::operators;

namespace smtk
{
namespace session
{
namespace polygon
{

bool CreateFacesFromEdges::populateEdgeMap()
{
  auto modelItem = this->parameters()->associations();
  smtk::model::Model model;

  internal::pmodel::Ptr storage; // Look up from session = internal::pmodel::create();

  // First, collect  edges to process:
  for (int i = 0; i < static_cast<int>(modelItem->numberOfValues()); ++i)
  {
    smtk::model::Edge edgeIn(modelItem->valueAs<smtk::model::Entity>(i));
    if (edgeIn.isValid())
    {
      if (model.isValid())
      {
        if (model != edgeIn.owningModel())
        {
          smtkErrorMacro(
            this->log(),
            "Edges from different models (" << model.name() << " and "
                                            << edgeIn.owningModel().name() << ") selected.");
          m_result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
          return false;
        }
      }
      else
      {
        model = edgeIn.owningModel();
      }
      m_edgeMap[edgeIn] = 0;
    }
  }
  if (m_edgeMap.empty() || !model.isValid())
  {
    smtkErrorMacro(this->log(), "No edges selected or invalid model specified.");
    m_result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
    return false;
  }
  m_model = model;
  return true;
}

const char* CreateFacesFromEdges::xmlDescription() const
{
  return CreateFacesFromEdges_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk

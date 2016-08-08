//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_CreateFacesFromEdges_h
#define __smtk_session_polygon_CreateFacesFromEdges_h

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/internal/Fragment.h" // for various internal types help by CreateFacesFromEdges
#include "smtk/bridge/polygon/internal/Neighborhood.h" // for various internal types help by CreateFacesFromEdges

#include "smtk/model/Face.h"

#include "smtk/common/UnionFind.h"

#include <map>
#include <set>
#include <vector>

namespace smtk {
  namespace bridge {
    namespace polygon {

class ActiveFragmentTree;
class Neighborhood;

/**\brief Create a face given a set of point coordinates or edges (but not both).
  *
  */
class SMTKPOLYGONSESSION_EXPORT CreateFacesFromEdges : public Operator
{
public:
  smtkTypeMacro(CreateFacesFromEdges);
  smtkCreateMacro(CreateFacesFromEdges);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  friend class Neighborhood;

  virtual smtk::model::OperatorResult operateInternal();

  void evaluateLoop(RegionId faceNumber, OrientedEdges& loop, std::set<RegionId>& borders);
  void addTessellations();

  std::map<RegionId, smtk::model::Face> m_regionFaces;
  std::map<RegionId, std::vector<OrientedEdges> > m_regionLoops;
  smtk::model::OperatorResult m_result;
  smtk::model::Model m_model;
  smtk::model::OperatorOutcome m_status;
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CreateFacesFromEdges_h

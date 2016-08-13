//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_CreateFaces_h
#define __smtk_session_polygon_CreateFaces_h

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/internal/Fragment.h" // for various internal types help by CreateFaces
#include "smtk/bridge/polygon/internal/Neighborhood.h" // for various internal types help by CreateFaces

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

/// An internal structure used when discovering edge loops.
struct SMTKPOLYGONSESSION_EXPORT ModelEdgeInfo
{
  ModelEdgeInfo()
    : m_allowedOrientations(0)
    {
    this->m_visited[0] = this->m_visited[1] = false;
    }
  ModelEdgeInfo(int allowedOrientations)
    {
    this->m_allowedOrientations = allowedOrientations > 0 ? +1 : allowedOrientations < 0 ? -1 : 0;
    this->m_visited[0] = this->m_visited[1] = false;
    }
  ModelEdgeInfo(const ModelEdgeInfo& other)
    : m_allowedOrientations(other.m_allowedOrientations)
    {
    for (int i = 0; i < 2; ++i)
      m_visited[i] = other.m_visited[i];
    }

  int m_allowedOrientations; // 0: all, -1: only negative, +1: only positive
  bool m_visited[2]; // has the [0]: negative, [1]: positive orientation of the edge been visited already?
};

/// An internal structure used to map model edges to information about the space between them.
typedef std::map<smtk::model::Edge, ModelEdgeInfo> ModelEdgeMap;

/**\brief Create a face given a set of point coordinates or edges (but not both).
  *
  */
class SMTKPOLYGONSESSION_EXPORT CreateFaces : public Operator
{
public:
  smtkTypeMacro(CreateFaces);
  smtkCreateMacro(CreateFaces);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  friend class Neighborhood;

  virtual bool populateEdgeMap();
  virtual smtk::model::OperatorResult operateInternal();

  void evaluateLoop(RegionId faceNumber, OrientedEdges& loop, std::set<RegionId>& borders);
  void updateLoopVertices(const smtk::model::Loop& loop, const smtk::model::Face& brd, bool isCCW);
  void addTessellations();

  std::map<RegionId, smtk::model::Face> m_regionFaces;
  std::map<RegionId, std::vector<OrientedEdges> > m_regionLoops;
  smtk::model::OperatorResult m_result;
  smtk::model::Model m_model;
  smtk::model::OperatorOutcome m_status;
  ModelEdgeMap m_edgeMap;
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CreateFaces_h

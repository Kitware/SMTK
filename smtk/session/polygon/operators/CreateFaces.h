//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_polygon_CreateFaces_h
#define smtk_session_polygon_CreateFaces_h

#include "smtk/session/polygon/Operation.h"
#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/Fragment.h" // for various internal types help by CreateFaces
#include "smtk/session/polygon/internal/Neighborhood.h" // for various internal types help by CreateFaces

#include "smtk/model/Face.h"

#include "smtk/common/UnionFind.h"

#include <map>
#include <set>
#include <vector>

namespace smtk
{
namespace session
{
namespace polygon
{

class ActiveFragmentTree;
class Neighborhood;

/// An internal structure used when discovering edge loops.
struct SMTKPOLYGONSESSION_EXPORT ModelEdgeInfo
{
  ModelEdgeInfo() { m_visited[0] = m_visited[1] = false; }
  ModelEdgeInfo(int allowedOrientations)
  {
    m_allowedOrientations = allowedOrientations > 0 ? +1 : allowedOrientations < 0 ? -1 : 0;
    m_visited[0] = m_visited[1] = false;
  }
  ModelEdgeInfo(const ModelEdgeInfo& other)
    : m_allowedOrientations(other.m_allowedOrientations)
  {
    for (int i = 0; i < 2; ++i)
      m_visited[i] = other.m_visited[i];
  }
  ModelEdgeInfo& operator=(const ModelEdgeInfo&) = default;

  int m_allowedOrientations{ 0 }; // 0: all, -1: only negative, +1: only positive
  bool m_visited
    [2]; // has the [0]: negative, [1]: positive orientation of the edge been visited already?
};

/// An internal structure used to map model edges to information about the space between them.
typedef std::map<smtk::model::Edge, ModelEdgeInfo> ModelEdgeMap;

/**\brief Create a face given a set of point coordinates or edges (but not both).
  *
  */
class SMTKPOLYGONSESSION_EXPORT CreateFaces : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::CreateFaces);
  smtkCreateMacro(CreateFaces);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  friend class Neighborhood;

  virtual bool populateEdgeMap();
  Result operateInternal() override;
  const char* xmlDescription() const override;

  void evaluateLoop(RegionId faceNumber, OrientedEdges& loop, std::set<RegionId>& borders);
  void updateLoopVertices(const smtk::model::Loop& loop, const smtk::model::Face& brd, bool isCCW);
  void removeFacesFromResult(const smtk::model::EntityRefs& faces);
  void addTessellations();

  std::map<RegionId, smtk::model::Face> m_regionFaces;
  std::map<RegionId, std::vector<OrientedEdges>> m_regionLoops;
  Result m_result;
  smtk::model::Model m_model;
  smtk::session::polygon::Resource::Ptr m_resource;
  Outcome m_status;
  ModelEdgeMap m_edgeMap;
  internal::Point m_bdsLo;
  internal::Point m_bdsHi;
};

} // namespace polygon
} //namespace session
} // namespace smtk

#endif // smtk_session_polygon_CreateFaces_h

//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_CleanGeometry_h
#define __smtk_session_polygon_CleanGeometry_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Create one or more edges given a set of point coordinates.
  *
  * Self-intersecting edges are broken into multiple non-self-intersecting edges.
  */
class SMTKPOLYGONSESSION_EXPORT CleanGeometry : public Operator
{
public:
  smtkTypeMacro(CleanGeometry);
  smtkCreateMacro(CleanGeometry);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();

  template<typename T, typename U, typename V, typename W, typename X>
  bool splitEdgeAsNeeded(
    const smtk::model::Edge& curEdge,
    internal::EdgePtr storage,
    T& result,
    U& lkup,
    V& rlkup,
    W& reslkup,
    X& endpoints,
    smtk::model::EntityRefArray& created,
    smtk::model::EntityRefArray& modified,
    smtk::model::EntityRefArray& expunged);

  template<typename T>
  void addVertex(const smtk::model::Vertex& vertex, T& pointMap);

  template<typename T>
  void addEdgePoints(const smtk::model::Edge& edge, T& pointMap);

  template<typename T>
  void addEdgeEndpoints(const smtk::model::Edge& edge, T& pointMap);

  template<typename T, typename U, typename V>
  void addDeferredSplits(const smtk::model::Edge& edge, T& actions, U& allpoints, U& endpoints, V& created);

  template<typename T, typename U, typename V, typename W>
  bool applyDeferredSplit(T mgr, U& action, V& allpoints, W& created);

  template<typename T, typename U>
  bool deleteIfDuplicates(T& edgePair, U& modified, U& expunged);
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CleanGeometry_h

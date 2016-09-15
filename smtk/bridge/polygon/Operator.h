//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_polygon_Operator_h
#define __smtk_session_polygon_Operator_h

#include "smtk/bridge/polygon/Session.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Manager.h"

/// A convenience macro for generating debug log messages.
#define smtkOpDebug(x) \
  if (this->m_debugLevel > 0) \
    { \
    smtkDebugMacro(this->log(), x ); \
    }

namespace smtk {
  namespace bridge {
    namespace polygon {

class Session;

/**\brief An operator using the polygon kernel.
  *
  * This is a base class for actual polygon operators.
  * It provides convenience methods for accessing polygon-specific data
  * for its subclasses to use internally.
  */
class SMTKPOLYGONSESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  Session* polygonSession();

  void addStorage(const smtk::common::UUID& uid, smtk::bridge::polygon::internal::entity::Ptr storage);
  bool removeStorage(const smtk::common::UUID& uid);
  template<typename T>
  typename T::Ptr findStorage(const smtk::common::UUID& uid)
    { return this->polygonSession()->findStorage<T>(uid); }
  template<typename T>
  T findOrAddStorage(const smtk::common::UUID& uid)
    { return this->polygonSession()->findOrAddStorage<T>(uid); }
  int nextModelNumber()
    { return this->polygonSession()->m_nextModelNumber++; }
  /*
  internal::Entity* polygonEntity(const smtk::model::EntityRef& smtkEntity);

  template<typename T>
  T polygonEntityAs(const smtk::model::EntityRef& smtkEntity);
  */

  template<typename T, typename U, typename V>
  void pointsForLoop(T& polypts, int numPtsToUse, U& start, U finish, int numCoordsPerPoint, V pmodel);

  template<typename T, typename U, typename V, typename W>
  void pointsForLoop(T& polypts, int numEdgesToUse, U& curEdge, U edgesFinish, V& curEdgeDir, V edgeDirFinish, W& outerLoopEdges);

  template<typename T, typename U>
    void pointsInLoopOrderFromOrientedEdges(T& polypts, U begin, U end, smtk::shared_ptr<internal::pmodel> pmodel);
};

/*
/// A convenience method for returning the polygon counterpart of an SMTK entity already cast to a subtype.
template<typename T>
T Operator::polygonEntityAs(const smtk::model::EntityRef& smtkEntity)
{
  return dynamic_cast<T>(this->polygonEntity(smtkEntity));
}
*/

    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Operator_h

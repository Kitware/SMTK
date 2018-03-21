//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_polygon_Operation_h
#define __smtk_session_polygon_Operation_h

#include "smtk/bridge/polygon/Exports.h"
#include "smtk/operation/XMLOperation.h"

#include "smtk/bridge/polygon/internal/Entity.h"

/// A convenience macro for generating debug log messages.
#define smtkOpDebug(x)                                                                             \
  if (m_debugLevel > 0)                                                                            \
  {                                                                                                \
    smtkDebugMacro(this->log(), x);                                                                \
  }

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief An operator using the polygon kernel.
  *
  * This is a base class for actual polygon operators.
  * It provides convenience methods for accessing polygon-specific data
  * for its subclasses to use internally.
  */
class SMTKPOLYGONSESSION_EXPORT Operation : public smtk::operation::XMLOperation
{
protected:
  template <typename T, typename U, typename V>
  void pointsForLoop(
    T& polypts, int numPtsToUse, U& start, U finish, int numCoordsPerPoint, V pmodel);

  template <typename T, typename U, typename V, typename W>
  void pointsForLoop(T& polypts, int numEdgesToUse, U& curEdge, U edgesFinish, V& curEdgeDir,
    V edgeDirFinish, W& outerLoopEdges);

  template <typename T, typename U>
  void pointsInLoopOrderFromOrientedEdges(
    T& polypts, U begin, U end, smtk::shared_ptr<internal::pmodel> pmodel);
};

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Operation_h

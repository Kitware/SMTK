//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_CreateEdgeFromPoints_h
#define __smtk_session_polygon_CreateEdgeFromPoints_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Create one edge given a set of point coordinates.
  *
  * Self-intersecting edges are broken into multiple non-self-intersecting edges.
  */
class SMTKPOLYGONSESSION_EXPORT CreateEdgeFromPoints : public Operator
{
public:
  smtkTypeMacro(CreateEdgeFromPoints);
  smtkCreateMacro(CreateEdgeFromPoints);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

  /**\brief Create one edge given a set of point coordinates.
  *
  * This is the main c++ method that can be used without the 
  * smtk attribute interface.
  */
  smtk::model::OperatorResult process(
    std::vector<double>& pnts, int numCoordsPerPoint, smtk::model::Model& parentModel);

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CreateEdgeFromPoints_h

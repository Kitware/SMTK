//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_ForceCreateFace_h
#define __smtk_session_polygon_ForceCreateFace_h

#include "smtk/bridge/polygon/Operator.h"

#include "smtk/common/UnionFind.h"

#include <map>
#include <set>
#include <vector>

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Create a face given a set of point coordinates or edges (but not both).
  *
  */
class SMTKPOLYGONSESSION_EXPORT ForceCreateFace : public Operator
{
public:
  smtkTypeMacro(ForceCreateFace);
  smtkCreateMacro(ForceCreateFace);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

  // NB: These must match discrete indices of "construction method" in ForceCreateFace.sbt:
  enum ConstructionMethod
  {
    POINTS = 0,
    EDGES = 1
  };

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_ForceCreateFace_h

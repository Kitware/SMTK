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

#include "smtk/session/polygon/Operation.h"

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

/**\brief Create a face given a set of point coordinates or edges (but not both).
  *
  */
class SMTKPOLYGONSESSION_EXPORT ForceCreateFace : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::ForceCreateFace);
  smtkCreateMacro(ForceCreateFace);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  // NB: These must match discrete indices of "construction method" in ForceCreateFace.sbt:
  enum ConstructionMethod
  {
    POINTS = 0,
    EDGES = 1
  };

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace polygon
} //namespace session
} // namespace smtk

#endif // __smtk_session_polygon_ForceCreateFace_h

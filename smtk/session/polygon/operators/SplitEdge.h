//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_SplitEdge_h
#define __smtk_session_polygon_SplitEdge_h

#include "smtk/session/polygon/Operation.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Create one or more edges given a set of point coordinates.
  *
  * Self-intersecting edges are broken into multiple non-self-intersecting edges.
  */
class SMTKPOLYGONSESSION_EXPORT SplitEdge : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::SplitEdge);
  smtkCreateMacro(SplitEdge);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace polygon
} //namespace session
} // namespace smtk

#endif // __smtk_session_polygon_SplitEdge_h

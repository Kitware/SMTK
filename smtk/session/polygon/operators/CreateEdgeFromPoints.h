//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_polygon_CreateEdgeFromPoints_h
#define smtk_session_polygon_CreateEdgeFromPoints_h

#include "smtk/session/polygon/Operation.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Create one edge given a set of point coordinates.
  *
  * Self-intersecting edges are broken into multiple non-self-intersecting edges.
  */
class SMTKPOLYGONSESSION_EXPORT CreateEdgeFromPoints : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::CreateEdgeFromPoints);
  smtkCreateMacro(CreateEdgeFromPoints);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  /**\brief Create one edge given a set of point coordinates.
  *
  * This is the main c++ method that can be used without the
  * smtk attribute interface.
  */
  Result process(std::vector<double>& pnts, int numCoordsPerPoint, smtk::model::Model& parentModel);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace polygon
} //namespace session
} // namespace smtk

#endif // smtk_session_polygon_CreateEdgeFromPoints_h

//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_CreateFacesFromEdges_h
#define __smtk_session_polygon_CreateFacesFromEdges_h

#include "smtk/session/polygon/operators/CreateFaces.h"

namespace smtk
{
namespace session
{
namespace polygon
{

class ActiveFragmentTree;
class Neighborhood;

/**\brief Create a face given a set of point coordinates or edges (but not both).
  *
  */
class SMTKPOLYGONSESSION_EXPORT CreateFacesFromEdges : public CreateFaces
{
public:
  smtkTypeMacro(smtk::session::polygon::CreateFacesFromEdges);
  smtkCreateMacro(CreateFacesFromEdges);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(CreateFaces);

protected:
  bool populateEdgeMap() override;
  virtual const char* xmlDescription() const override;
};

} // namespace polygon
} //namespace session
} // namespace smtk

#endif // __smtk_session_polygon_CreateFacesFromEdges_h

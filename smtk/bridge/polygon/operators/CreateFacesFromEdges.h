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

#include "smtk/bridge/polygon/operators/CreateFaces.h"

namespace smtk
{
namespace bridge
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
  smtkTypeMacro(CreateFacesFromEdges);
  smtkCreateMacro(CreateFacesFromEdges);
  smtkSharedFromThisMacro(CreateFaces);
  smtkSuperclassMacro(CreateFaces);
  smtkDeclareModelOperator();

protected:
  virtual bool populateEdgeMap();
};

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CreateFacesFromEdges_h

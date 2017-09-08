//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_DemoteVertex_h
#define __smtk_session_polygon_DemoteVertex_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Create one or more edges given a set of point coordinates.
  *
  * Self-intersecting edges are broken into multiple non-self-intersecting edges.
  */
class SMTKPOLYGONSESSION_EXPORT DemoteVertex : public Operator
{
public:
  smtkTypeMacro(DemoteVertex);
  smtkCreateMacro(DemoteVertex);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_DemoteVertex_h

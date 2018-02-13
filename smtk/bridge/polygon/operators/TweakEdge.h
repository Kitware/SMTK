//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_TweakEdge_h
#define __smtk_session_polygon_TweakEdge_h

#include "smtk/bridge/polygon/Operation.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Tweak the shape of a model edge by redefining its point coordinates.
  */
class SMTKPOLYGONSESSION_EXPORT TweakEdge : public Operation
{
public:
  smtkTypeMacro(TweakEdge);
  smtkCreateMacro(TweakEdge);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_TweakEdge_h

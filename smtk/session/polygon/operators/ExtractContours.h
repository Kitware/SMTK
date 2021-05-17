//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_polygon_ExtractContours_h
#define __smtk_session_polygon_ExtractContours_h

#include "smtk/session/polygon/Operation.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Create polygon edges from contours extraced from an image.
  *
  * This operator will eate polygon edges from contours extraced from
  * an image associated with a model. It interacts with the application
  * through a view "UIConstructor", specified in the operator's template
  * file.
  *
  */
class SMTKPOLYGONSESSION_EXPORT ExtractContours : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::ExtractContours);
  smtkCreateMacro(ExtractContours);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // __smtk_session_polygon_ExtractContours_h

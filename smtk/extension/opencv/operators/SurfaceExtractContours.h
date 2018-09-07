//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_opencv_SurfaceExtractContours_h
#define __smtk_extension_opencv_SurfaceExtractContours_h

#include "smtk/extension/opencv/Exports.h"
#include "smtk/session/polygon/Operation.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Create polygon edges from contours extraced from an image.
  *
  * This operator will create polygon edges from contours extraced from
  * an image associated with a model. It interacts with the application
  * through a view "UIConstructor", specified in the operator's template
  * file.
  *
  */
class SMTKOPENCVEXT_EXPORT SurfaceExtractContours : public Operation
{
public:
  smtkTypeMacro(SurfaceExtractContours);
  smtkCreateMacro(SurfaceExtractContours);
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

#endif

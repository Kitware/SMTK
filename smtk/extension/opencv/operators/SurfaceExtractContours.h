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

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/extension/opencv/Exports.h"

namespace smtk
{
namespace bridge
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
class SMTKOPENCVEXT_EXPORT SurfaceExtractContours : public Operator
{
public:
  smtkTypeMacro(SurfaceExtractContours);
  smtkCreateMacro(SurfaceExtractContours);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif

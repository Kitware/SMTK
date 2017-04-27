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

#include "smtk/bridge/polygon/Operator.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Create polygon edges from contours extraced from an image.
  *
  * This operator will eate polygon edges from contours extraced from
  * an image assoicated with a model. It interacts with the application
  * through a view "UIConstructor", specified in the operator's template
  * file.
  *
  */
class SMTKPOLYGONSESSION_EXPORT ExtractContours : public Operator
{
public:
  smtkTypeMacro(ExtractContours);
  smtkCreateMacro(ExtractContours);
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

#endif // __smtk_session_polygon_ExtractContours_h

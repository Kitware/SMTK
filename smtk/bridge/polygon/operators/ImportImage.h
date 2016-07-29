//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_polygon_ImportImage_h
#define __smtk_session_polygon_ImportImage_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Import an image file to a polygon model.
  *
  * The supported file extensions currently:
  *    "Image files (*.vti *.tif);;DEM(*.dem)
  */
class SMTKPOLYGONSESSION_EXPORT ImportImage : public Operator
{
public:
  smtkTypeMacro(ImportImage);
  smtkCreateMacro(ImportImage);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  ImportImage();
  virtual smtk::model::OperatorResult operateInternal();

};

    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_ImportImage_h

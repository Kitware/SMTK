//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_polygon_EditEdge_h
#define __smtk_session_polygon_EditEdge_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Edit an edge in a polygon model file.
  *
  * This operator allows to create and modify an edge
  * in a polygon model. It interacts with the application
  * through a view "UIConstructor", specified in the operator's template
  * file.
  *
  */
class SMTKPOLYGONSESSION_EXPORT EditEdge : public Operator
{
public:
  smtkTypeMacro(EditEdge);
  smtkCreateMacro(EditEdge);
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

#endif // __smtk_session_polygon_EditEdge_h

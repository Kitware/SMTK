//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_SetProperty_h
#define __smtk_mesh_SetProperty_h

#include "smtk/model/Operator.h"

namespace smtk {
  namespace mesh {

class SMTKCORE_EXPORT SetProperty : public smtk::model::Operator
{
public:
  smtkTypeMacro(SetProperty);
  smtkCreateMacro(SetProperty);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

  } //namespace mesh
} // namespace smtk

#endif // __smtk_mesh_SetProperty_h

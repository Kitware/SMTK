//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_InterpolateMesh_h
#define __smtk_mesh_operators_InterpolateMesh_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for generating a mesh data set from interpolation points.
  */
class SMTKCORE_EXPORT InterpolateMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(InterpolateMesh);
  smtkCreateMacro(InterpolateMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};
}
}

#endif // __smtk_mesh_operators_InterpolateMesh_h

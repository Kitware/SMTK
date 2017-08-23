//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_UndoWarpMesh_h
#define __smtk_mesh_operators_UndoWarpMesh_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for reversing the results of operators that warp a mesh.

   Operators that use the methods defined in ApplyToMesh to deform a mesh's
   coordinates can optionally cache the original unwarped coordinate values.
   This operator then reverts the deformation performed by these operators.
  */
class SMTKCORE_EXPORT UndoWarpMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(UndoWarpMesh);
  smtkCreateMacro(UndoWarpMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  smtk::model::OperatorResult operateInternal() override;
};
}
}

#endif // __smtk_mesh_operators_UndoWarpMesh_h

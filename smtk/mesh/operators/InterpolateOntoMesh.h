//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_InterpolateOntoMesh_h
#define __smtk_mesh_operators_InterpolateOntoMesh_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for generating a mesh data set from interpolation points.
  */
class SMTKCORE_EXPORT InterpolateOntoMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(InterpolateOntoMesh);
  smtkCreateMacro(InterpolateOntoMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  smtk::model::OperatorResult operateInternal() override;
};
}
}

#endif // __smtk_mesh_operators_InterpolateOntoMesh_h

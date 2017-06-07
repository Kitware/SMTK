//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_ElevateMesh_h
#define __smtk_mesh_operators_ElevateMesh_h

#include "smtk/extension/vtk/operators/Exports.h" // For export macro
#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief
  */
class VTKSMTKOPERATORSEXT_EXPORT ElevateMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(ElevateMesh);
  smtkCreateMacro(ElevateMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};
}
}

#endif // __smtk_mesh_operators_ElevateMesh_h

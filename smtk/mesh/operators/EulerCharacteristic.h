//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_EulerCharacteristic_h
#define __smtk_mesh_EulerCharacteristic_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief Compute and return the Euler characteristic for a mesh.
  */
class SMTKCORE_EXPORT EulerCharacteristic : public smtk::model::Operator
{
public:
  smtkTypeMacro(EulerCharacteristic);
  smtkCreateMacro(EulerCharacteristic);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // mesh namespace
} // smtk namespace

#endif // __smtk_mesh_EulerCharacteristic_h

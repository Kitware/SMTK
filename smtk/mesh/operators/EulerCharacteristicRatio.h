//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_EulerCharacteristicRatio_h
#define __smtk_mesh_EulerCharacteristicRatio_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief Compute and return the ratio of Euler characteristic surface to volume
   for a model's mesh tessellation.
  */
class SMTKCORE_EXPORT EulerCharacteristicRatio : public smtk::model::Operator
{
public:
  smtkTypeMacro(EulerCharacteristicRatio);
  smtkCreateMacro(EulerCharacteristicRatio);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace mesh
} // namespace smtk

#endif // __smtk_mesh_EulerCharacteristicRatio_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_mesh_EulerCharacteristicRatio_h
#define __smtk_bridge_mesh_EulerCharacteristicRatio_h

#include "smtk/bridge/mesh/Operator.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

/**\brief Compute and return the ratio of Euler characteristic surface to volume
   for a model's mesh tessellation.
  */
class SMTKMESHSESSION_EXPORT EulerCharacteristicRatio : public Operator
{
public:
  smtkTypeMacro(EulerCharacteristicRatio);
  smtkCreateMacro(EulerCharacteristicRatio);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace mesh
} // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_mesh_EulerCharacteristicRatio_h

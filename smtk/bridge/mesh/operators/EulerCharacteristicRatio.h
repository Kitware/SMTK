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

#include "smtk/bridge/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

/**\brief Compute and return the ratio of Euler characteristic surface to volume
   for a model's mesh tessellation.
  */
class SMTKMESHSESSION_EXPORT EulerCharacteristicRatio : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(EulerCharacteristicRatio);
  smtkCreateMacro(EulerCharacteristicRatio);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace mesh
} // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_mesh_EulerCharacteristicRatio_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_SetMeshName_h
#define __smtk_mesh_SetMeshName_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Set the name of a meshset.
  */
class SMTKCORE_EXPORT SetMeshName : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::SetMeshName);
  smtkCreateMacro(SetMeshName);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} //namespace mesh
} // namespace smtk

#endif // __smtk_mesh_SetMeshName_h

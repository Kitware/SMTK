//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_DeleteMesh_h
#define __smtk_mesh_DeleteMesh_h

#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace mesh
{

/**\brief Delete a mesh.
  */
class SMTKCORE_EXPORT DeleteMesh : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(DeleteMesh);
  smtkCreateMacro(DeleteMesh);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::XMLOperator);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} //namespace mesh
} // namespace smtk

#endif // __smtk_mesh_DeleteMesh_h

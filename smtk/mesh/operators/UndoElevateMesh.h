//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_UndoElevateMesh_h
#define __smtk_mesh_operators_UndoElevateMesh_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for reversing the results of operators that elevate a mesh.

   Operations that use the methods defined in ApplyToMesh to deform a mesh's
   coordinates can optionally cache the original unelevated coordinate values.
   This operator then reverts the deformation performed by these operators.
  */
class SMTKCORE_EXPORT UndoElevateMesh : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::UndoElevateMesh);
  smtkCreateMacro(UndoElevateMesh);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
} // namespace mesh
} // namespace smtk

#endif // __smtk_mesh_operators_UndoElevateMesh_h

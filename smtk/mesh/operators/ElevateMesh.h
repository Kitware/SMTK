//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_operators_ElevateMesh_h
#define smtk_mesh_operators_ElevateMesh_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for modifying the z-coordinates of a mesh's nodes according
   to an interpolated data set.

   Given a data set of either structured or unstructured data, an interpolation
   scheme and input mesh, the z-coordinate of each point in the input mesh is
   set to the interpolated values in the external data set. The resulting mesh
   deformation can be undone by subsequently applying the "Undo Elevate" filter,
   returning the mesh nodes to their original positions.
  */
class SMTKCORE_EXPORT ElevateMesh : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::ElevateMesh);
  smtkCreateMacro(ElevateMesh);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace mesh
} // namespace smtk

#endif // smtk_mesh_operators_ElevateMesh_h

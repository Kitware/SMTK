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

/**\brief A class for modifying the z-coordinates of a mesh's nodes according
   to a VTK data set.

   Given a VTK-compatible data set of either structured or unstructured data and
   an input mesh, the z-coordinate of each point in the input mesh is set to the
   average of the values in the external data set whose spatial coordinates lie
   within a user-defined radius of the projection of the mesh point onto the
   dataset. The resulting mesh deformation can be undone by subsequently
   applying the "Undo Warp" filter, returning the mesh nodes to their original
   position.
  */
class VTKSMTKOPERATORSEXT_EXPORT ElevateMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(ElevateMesh);
  smtkCreateMacro(ElevateMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  smtk::model::OperatorResult operateInternal() override;
};
}
}

#endif // __smtk_mesh_operators_ElevateMesh_h

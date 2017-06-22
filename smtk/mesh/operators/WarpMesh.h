//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_WarpMesh_h
#define __smtk_mesh_operators_WarpMesh_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for modifying the z-coordinates of a mesh's nodes according
   to an external data set.

   Given an external data set of either structured or unstructured data and an
   input mesh, the z-coordinate of each point in the input mesh is set to the
   inverse distance weighted average of the values in the external data. The
   resulting mesh deformation can be undone by subsequently applying the "Undo
   Warp" filter, returning the mesh nodes to their original position.
  */
class SMTKCORE_EXPORT WarpMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(WarpMesh);
  smtkCreateMacro(WarpMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};
}
}

#endif // __smtk_mesh_operators_WarpMesh_h

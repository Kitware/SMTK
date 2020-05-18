//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_InterpolateOntoMesh_h
#define __smtk_mesh_operators_InterpolateOntoMesh_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for generating a mesh data set from interpolation points.
  */
class SMTKCORE_EXPORT InterpolateOntoMesh : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::InterpolateOntoMesh);
  smtkCreateMacro(InterpolateOntoMesh);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
} // namespace mesh
} // namespace smtk

#endif // __smtk_mesh_operators_InterpolateOntoMesh_h

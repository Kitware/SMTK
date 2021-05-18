//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_ExtractAdjacency_h
#define smtk_mesh_ExtractAdjacency_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Extract a mesh's adjacency mesh.

   The user can select the dimension of the returned adjacency mesh.
  */
class SMTKCORE_EXPORT ExtractAdjacency : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::ExtractAdjacency);
  smtkCreateMacro(ExtractAdjacency);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace mesh
} // namespace smtk

#endif

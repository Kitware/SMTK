//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_delaunay_TriangulateFaces_h
#define __smtk_extension_delaunay_TriangulateFaces_h

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace extension
{
namespace delaunay
{

class Session;

/**\brief Triangulate model faces into meshes using Delaunay.
  *
  * This operation creates smtk::mesh::MeshSets associated with
  * smtk::mesh::Faces using Delaunay. The MeshSets reside in the
  * smtk::mesh::Resource with the same UUID as the Faces' model. If this
  * resource does not yet exist during the construction of the meshes, it is
  * created and populated with the MeshSets.
  */
class SMTKDELAUNAYEXT_EXPORT TriangulateFaces : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(TriangulateFaces);
  smtkCreateMacro(TriangulateFaces);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  TriangulateFaces();
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
}
}
}

#endif

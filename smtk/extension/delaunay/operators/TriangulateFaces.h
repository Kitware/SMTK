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
#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

class Session;

/**\brief Triangulate model faces into meshes using Delaunay.
  *
  * This operation creates smtk::mesh::MeshSets associated with
  * smtk::mesh::Faces using Delaunay. The MeshSets reside in the
  * smtk::mesh::Collection with the same UUID as the Faces' model. If this
  * collection does not yet exist during the construction of the meshes, it is
  * created and populated with the MeshSets.
  */
class SMTKDELAUNAYEXT_EXPORT TriangulateFaces : public smtk::model::Operator
{
public:
  smtkTypeMacro(TriangulateFaces);
  smtkSuperclassMacro(Operator);
  smtkCreateMacro(TriangulateFaces);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  TriangulateFaces();
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace model
} // namespace smtk

#endif // __smtk_extension_delaunay_TriangulateFaces_h

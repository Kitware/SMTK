//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_delaunay_TriangulateFace_h
#define __smtk_extension_delaunay_TriangulateFace_h

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

class Session;

/**\brief Triangulate a model face into a mesh using Delaunay.
  *
  * This operation creates an smtk::mesh::MeshSet associated with an
  * smtk::mesh::Face using Delaunay. The MeshSet resides in the
  * smtk::mesh::Collection with the same UUID as the Face's model. If this
  * collection does not yet exist during the construction of the mesh, it is
  * created and populated with the MeshSet.
  */
class SMTKDELAUNAYEXT_EXPORT TriangulateFace : public Operator
{
public:
  smtkTypeMacro(TriangulateFace);
  smtkSuperclassMacro(Operator);
  smtkCreateMacro(TriangulateFace);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  TriangulateFace();
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace model
} // namespace smtk

#endif // __smtk_extension_delaunay_TriangulateFace_h

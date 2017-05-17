//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_WriteMesh_h
#define __smtk_mesh_WriteMesh_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for writing meshes to file.
  */
class SMTKCORE_EXPORT WriteMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(WriteMesh);
  smtkCreateMacro(WriteMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // mesh namespace
} // smtk namespace

#endif // __smtk_mesh_WriteMesh_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_ExportMesh_h
#define __smtk_mesh_ExportMesh_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for writing meshes to file.
  */
class SMTKCORE_EXPORT ExportMesh : public smtk::model::Operator
{
public:
  smtkTypeMacro(ExportMesh);
  smtkCreateMacro(ExportMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // mesh namespace
} // smtk namespace

#endif // __smtk_mesh_ExportMesh_h

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

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for writing meshes to file.
  */
class SMTKCORE_EXPORT ExportMesh : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(ExportMesh);
  smtkCreateMacro(ExportMesh);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // mesh namespace
} // smtk namespace

#endif // __smtk_mesh_ExportMesh_h

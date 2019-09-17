//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_ExtractSkin_h
#define smtk_mesh_ExtractSkin_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Extract a mesh's skin.
  */
class SMTKCORE_EXPORT ExtractSkin : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::ExtractSkin);
  smtkCreateMacro(ExtractSkin);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
}
}

#endif

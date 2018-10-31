//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_Import_h
#define __smtk_mesh_Import_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Import an smtk mesh file.
 *
 * Here and throughout SMTK, we use the terms read/write to describe
 * serialization of native SMTK files, while the terms import/export describe
 * transcription from/to a different format.
 */
class SMTKCORE_EXPORT Import : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
  virtual const char* xmlDescription() const override;
};
}
}

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_WriteResource_h
#define smtk_mesh_WriteResource_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Write an smtk mesh file.
  */
class SMTKCORE_EXPORT WriteResource : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::WriteResource);
  smtkCreateMacro(WriteResource);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKCORE_EXPORT bool write(
  const smtk::resource::ResourcePtr&,
  const std::shared_ptr<smtk::common::Managers>&);
} // namespace mesh
} // namespace smtk

#endif

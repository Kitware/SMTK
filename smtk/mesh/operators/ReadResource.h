//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_ReadResource_h
#define __smtk_mesh_ReadResource_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Read an smtk mesh resource.
 */
class SMTKCORE_EXPORT ReadResource : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::ReadResource);
  smtkCreateMacro(ReadResource);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKCORE_EXPORT smtk::resource::ResourcePtr read(const std::string&);
} // namespace mesh
} // namespace smtk

#endif

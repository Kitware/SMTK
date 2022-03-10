//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Write_h
#define smtk_project_Write_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

/**\brief Write a project, including its resources, to the file system.

  This operator writes the selected project to the file system along
  with its resources. Because the project itself is an SMTK resource,
  it uses the standard .smtk extension. Resources contained by the
  project are written to a "resources" subdirectory.
  */
class SMTKCORE_EXPORT Write : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Write);
  smtkCreateMacro(Write);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

SMTKCORE_EXPORT bool write(
  const smtk::resource::ResourcePtr&,
  const std::shared_ptr<smtk::common::Managers>&);
} // namespace project
} // namespace smtk

#endif

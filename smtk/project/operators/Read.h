//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Read_h
#define smtk_project_Read_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

/**\brief Read project from file system.

  This operator reads a project file and all of its
  resource files from disk.
  */
class SMTKCORE_EXPORT Read : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  void markModifiedResources(Result&) override;
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

SMTKCORE_EXPORT smtk::resource::ResourcePtr read(
  const std::string&,
  const std::shared_ptr<smtk::common::Managers>&);
} // namespace project
} // namespace smtk

#endif

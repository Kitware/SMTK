//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_project_CreateProject_h
#define __smtk_project_CreateProject_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

class SMTKCORE_EXPORT CreateProject : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::CreateProject);
  smtkCreateMacro(CreateProject);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
  virtual const char* xmlDescription() const override;
};
} // namespace project
} // namespace smtk

#endif

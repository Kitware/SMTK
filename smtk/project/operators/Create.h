//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_project_Create_h
#define __smtk_project_Create_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

/**\brief Create a project instance of specified type.

  Create a project instance. The project type must be one that
  has been registered with the project manager.
  */
class SMTKCORE_EXPORT Create : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Create);
  smtkCreateMacro(Create);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
  virtual const char* xmlDescription() const override;
};
} // namespace project
} // namespace smtk

#endif

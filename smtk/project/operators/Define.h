//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_project_Define_h
#define __smtk_project_Define_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

/**\brief Define a basic project type.

  Register a new project type with the project manager.
  Options are available to white-list associated resources
  and operations.
  */
class SMTKCORE_EXPORT Define : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Define);
  smtkCreateMacro(Define);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
  const char* xmlDescription() const override;
};
} // namespace project
} // namespace smtk

#endif

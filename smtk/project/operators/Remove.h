//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Remove_h
#define smtk_project_Remove_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

/**\brief Remove a resource from a project.
  */
class SMTKCORE_EXPORT Remove : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Remove);
  smtkCreateMacro(Remove);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace project
} // namespace smtk

#endif

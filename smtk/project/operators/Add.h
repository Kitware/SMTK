//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Add_h
#define smtk_project_Add_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

/**\brief Add a resource to a project.

    Add a resource with an optional role to a project.
    If the project includes a resource whitelist, the
    resource type must be in the whitelist.
  */
class SMTKCORE_EXPORT Add : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Add);
  smtkCreateMacro(Add);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  bool configure(const smtk::attribute::AttributePtr&, const smtk::attribute::ItemPtr&) override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace project
} // namespace smtk

#endif

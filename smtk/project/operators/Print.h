//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_project_Print_h
#define __smtk_project_Print_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

class SMTKCORE_EXPORT Print : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Print);
  smtkCreateMacro(Print);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
} // namespace project
} // namespace smtk

#endif

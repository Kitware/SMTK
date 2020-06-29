//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_project_Read_h
#define __smtk_project_Read_h

#include "smtk/project/Operation.h"

namespace smtk
{
namespace project
{

class SMTKCORE_EXPORT Read : public smtk::project::Operation
{
public:
  smtkTypeMacro(smtk::project::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

SMTKCORE_EXPORT smtk::resource::ResourcePtr read(const std::string&);
} // namespace project
} // namespace smtk

#endif

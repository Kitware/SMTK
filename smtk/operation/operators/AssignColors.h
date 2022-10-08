//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_AssignColors_h
#define smtk_operation_AssignColors_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

class SMTKCORE_EXPORT AssignColors : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::AssignColors);
  smtkCreateMacro(AssignColors);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} //namespace operation
} // namespace smtk

#endif // smtk_operation_AssignColors_h

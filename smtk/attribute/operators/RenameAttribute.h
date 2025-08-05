//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_operators_RenameAttribute_h
#define smtk_operation_operators_RenameAttribute_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief An operation to create an attribute according to a concrete definition.
  */
class SMTKCORE_EXPORT RenameAttribute : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::attribute::RenameAttribute);
  smtkCreateMacro(RenameAttribute);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  void generateSummary(Operation::Result&) override;
  const char* xmlDescription() const override;
};
} // namespace attribute
} // namespace smtk

#endif // smtk_operation_operators_RenameAttribute_h

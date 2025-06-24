//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_operators_DeleteAttribute_h
#define smtk_operation_operators_DeleteAttribute_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief An operation to delete an attribute.
  *
  * No checking is performed to ensure the attribute is unreferenced/unassociated
  * before deletion.
  */
class SMTKCORE_EXPORT DeleteAttribute : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::attribute::DeleteAttribute);
  smtkCreateMacro(DeleteAttribute);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  void generateSummary(Operation::Result&) override;
  const char* xmlDescription() const override;
};
} // namespace attribute
} // namespace smtk

#endif // smtk_operation_operators_DeleteAttribute_h

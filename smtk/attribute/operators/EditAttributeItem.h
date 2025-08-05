//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_operators_EditAttributeItem_h
#define smtk_operation_operators_EditAttributeItem_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief An operation to edit one item of one attribute.

   If the item is optional, it may be enabled/disabled.
   If the item is extensible, the number of values may be modified.
   If the item has values, those values may be modified.
   If any edits are performed, the operation succeeds and
   the corresponding attribute is marked modified.
  */
class SMTKCORE_EXPORT EditAttributeItem : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::attribute::EditAttributeItem);
  smtkCreateMacro(EditAttributeItem);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  void generateSummary(Operation::Result&) override;
  const char* xmlDescription() const override;
};
} // namespace attribute
} // namespace smtk

#endif // smtk_operation_operators_EditAttributeItem_h

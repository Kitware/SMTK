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

/**\brief A "dummy" operation used to mark an attribute as created, modified, or expunged.

   This operation does nothing internally except pass the components
   which are associated to itself into its result's "created", "modified",
   or "expunged" entries.
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

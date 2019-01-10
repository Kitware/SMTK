//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_operators_MarkModified_h
#define smtk_operation_operators_MarkModified_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

/**\brief A "dummy" operation used to mark a component's resource as modified.

   This operation does nothing internally except pass the components
   which are associated to itself into its result's "modified" entry.
   That will result in their owning resources being marked as
   dirty so that the user interface can respond to this change (by
   prompting the user to save when necessary).

   Since the operation is only intended for internal use, it is not
   registered with the operation manager.
  */
class SMTKCORE_EXPORT MarkModified : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::MarkModified);
  smtkCreateMacro(MarkModified);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  void generateSummary(Operation::Result&) override {}
  virtual const char* xmlDescription() const override;
};
}
}

#endif // smtk_operation_operators_MarkModified_h

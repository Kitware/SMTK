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

/**\brief A "dummy" operation used to mark a resource as modified.

   This operation does nothing internally except pass the resources
   associated to itself into its result's "resources" item.
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
  void generateSummary(Operation::Result&) override;
  const char* xmlDescription() const override;
};
} // namespace operation
} // namespace smtk

#endif // smtk_operation_operators_MarkModified_h

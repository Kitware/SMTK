//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operation_ImportResource_h
#define smtk_model_operation_ImportResource_h

#include "smtk/operation/Operation.h"

namespace smtk
{
namespace operation
{

/// Load an SMTK resource from a file.
class SMTKCORE_EXPORT ImportResource : public Operation
{
public:
  static constexpr const char* const file_item_name = "filename";

  smtkTypeMacro(smtk::operation::ImportResource);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::Operation);

  bool ableToOperate() override;

protected:
  ImportResource();

  Result operateInternal() override;

  Specification createSpecification() override;
  void generateSummary(Result&) override;
};
} // namespace operation
} // namespace smtk

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operation_ReadResource_h
#define smtk_model_operation_ReadResource_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

/// Load an SMTK resource from a file.
class SMTKCORE_EXPORT ReadResource : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::ReadResource);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  ReadResource();

  Result operateInternal() override;

  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
  void generateSummary(Result&) override;
};
} // namespace operation
} // namespace smtk

#endif

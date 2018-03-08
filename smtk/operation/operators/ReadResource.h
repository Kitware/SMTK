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
  smtkTypeMacro(ReadResource);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  ReadResource();

  virtual bool ableToOperate() override;

  Result operateInternal() override;

  virtual const char* xmlDescription() const override;
  void generateSummary(Result&) override;
};
}
}

#endif

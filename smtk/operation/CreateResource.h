//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operation_CreateResource_h
#define smtk_model_operation_CreateResource_h

#include "smtk/operation/ResourceManagerOperation.h"

namespace smtk
{
namespace operation
{

/// Create an SMTK resource.
class SMTKCORE_EXPORT CreateResource : public smtk::operation::ResourceManagerOperation
{
public:
  smtkTypeMacro(CreateResource);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::ResourceManagerOperation);

protected:
  CreateResource();

  Result operateInternal() override;

  virtual const char* xmlDescription() const override;
  void generateSummary(Result&) override;
};
}
}

#endif

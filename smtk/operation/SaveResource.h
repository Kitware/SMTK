//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_SaveResource_h
#define smtk_operation_SaveResource_h

#include "smtk/operation/ResourceManagerOperation.h"

namespace smtk
{
namespace operation
{

/// An operation that uses resource metadata to write resources.
class SMTKCORE_EXPORT SaveResource : public smtk::operation::ResourceManagerOperation
{
public:
  smtkTypeMacro(SaveResource);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  virtual bool ableToOperate() override;

protected:
  SaveResource();

  Result operateInternal() override;

  virtual const char* xmlDescription() const override;
  void generateSummary(Result&) override;
};
}
}

#endif

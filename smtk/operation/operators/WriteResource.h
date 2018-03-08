//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_WriteResource_h
#define smtk_operation_WriteResource_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

/// An operation that uses resource metadata to write resources.
class SMTKCORE_EXPORT WriteResource : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(WriteResource);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  WriteResource();

  virtual bool ableToOperate() override;

  Result operateInternal() override;

  virtual const char* xmlDescription() const override;
  void generateSummary(Result&) override;
};
}
}

#endif

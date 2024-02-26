//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_CopyResources_h
#define smtk_operation_CopyResources_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

/// An operation that uses resource metadata to write resources.
class SMTKCORE_EXPORT CopyResources : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::CopyResources);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  CopyResources();

  Result operateInternal() override;

  const char* xmlDescription() const override;
};

} // namespace operation
} // namespace smtk

#endif // smtk_operation_CopyResources_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_SetProperty_h
#define smtk_operation_SetProperty_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

/// Set (or remove) a property value on a set of entities. The string, integer,
/// and floating-point values are all optional. Any combination may be specified.
/// All that are specified are set; those unspecified are removed.
class SMTKCORE_EXPORT SetProperty : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::SetProperty);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace operation
} // namespace smtk

#endif

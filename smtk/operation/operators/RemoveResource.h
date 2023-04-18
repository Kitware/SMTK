//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operation_RemoveResource_h
#define smtk_model_operation_RemoveResource_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

/// Remove an SMTK resource from its resource manager. Removed resources will
/// still exist in memory for as long as the operation result is held.
/// Additionally, any other classes that hold a shared pointer to the resource
/// will keep the resource in memory.
class SMTKCORE_EXPORT RemoveResource : public XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::RemoveResource);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  /// Linked resources and attribute resources need this to
  /// run on the main thread to update the UI properly.
  bool threadSafe() const override { return false; }

protected:
  RemoveResource();

  bool ableToOperate() override;

  Result operateInternal() override;

  /// Do not report success as the base class prints specific console
  /// messages after operateInternal() completes.
  void generateSummary(Result&) override {}

  const char* xmlDescription() const override;
};
} // namespace operation
} // namespace smtk

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operation_ResourceManagerOperator_h
#define smtk_model_operation_ResourceManagerOperator_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace operation
{

/// A base class for operations that require access to a resource manager.
///
/// Operators that inherit from this class and that are created by an operation
/// manager that has a resource manager registered to it will have the resource
/// manager assigned to them upon creation. Otherwise, the resource manager must
/// be set manually.
class SMTKCORE_EXPORT ResourceManagerOperator : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(ResourceManagerOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::XMLOperator);

  void setResourceManager(smtk::resource::WeakManagerPtr);
  smtk::resource::ManagerPtr resourceManager();

  virtual bool ableToOperate() override;

private:
  smtk::resource::WeakManagerPtr m_resourceManager;
};
}
}

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operation_ResourceManagerOperation_h
#define smtk_model_operation_ResourceManagerOperation_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace operation
{

/// A base class for operations that require access to a resource manager.
///
/// Operations that inherit from this class and that are created by an operation
/// manager that has a resource manager registered to it will have the resource
/// manager assigned to them upon creation. Otherwise, the resource manager must
/// be set manually.
class SMTKCORE_EXPORT ResourceManagerOperation : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(ResourceManagerOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  smtk::resource::ManagerPtr resourceManager();
};
}
}

#endif

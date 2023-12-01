//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_RemoveDependency_h
#define smtk_task_RemoveDependency_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace task
{

/**\brief Remove a task dependency between a pair of tasks.
  *
  * Upon completion, both the upstream and downstream tasks
  * are marked modified.
  */
class SMTKCORE_EXPORT RemoveDependency : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::task::RemoveDependency);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkCreateMacro(RemoveDependency);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_RemoveDependency_h

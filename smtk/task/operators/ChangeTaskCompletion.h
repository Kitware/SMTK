//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_ChangeTaskCompletion_h
#define smtk_task_ChangeTaskCompletion_h

#include "smtk/operation/XMLOperation.h"

#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

/**\brief Change a task's completion status (either marking it completed or un-completed).
  *
  * This operation is called when a task's m_completed ivar should be modified
  * (either to true or false).
  * The operation will fail if no such change can be accomplished (either because the
  * task was already in the proper state or the proposed state change was prohibited.
  * The latter can occur when a user attempts to mark a task completed even though it
  * is not in a completable state.
  *
  * This operation should be used for all such transitions, whether user-prompted
  * or programmatically induced.
  */
class SMTKCORE_EXPORT ChangeTaskCompletion : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::task::ChangeTaskCompletion);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkCreateMacro(ChangeTaskCompletion);
  void setTask(const smtk::resource::Component::Ptr& task);
  void setCompleted(bool taskCompleted);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_ChangeTaskCompletion_h

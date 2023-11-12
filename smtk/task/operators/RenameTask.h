//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_RenameTask_h
#define smtk_task_RenameTask_h

#include "smtk/operation/XMLOperation.h"

#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

/**\brief Emplace (instantiate) a worklet into a project.
  *
  * This operation deserializes a worklet's JSON into the
  * it's project's task manager.
  */
class SMTKCORE_EXPORT RenameTask : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::task::RenameTask);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkCreateMacro(RenameTask);
  void setName(std::string& newName);
  void setTask(const Task::Ptr& task);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_RenameTask_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_job_operators_UpdateContainerQueueMachine_h
#define smtk_qt_job_operators_UpdateContainerQueueMachine_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace qt
{
namespace job
{

/**\brief An operation to stop and re-create a ContainerQueue's virtual machine.
  *
  * This job should be run when the ContainerQueue's rootJobDirectory is modified
  * so that the queue's VM can properly map host OS case directories into containers
  * run in the VM.
  */
class SMTKQTEXT_EXPORT UpdateContainerQueueMachine : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::qt::job::UpdateContainerQueueMachine);
  smtkCreateMacro(UpdateContainerQueueMachine);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace job
} // namespace qt
} // namespace smtk

#endif // smtk_qt_job_operators_UpdateContainerQueueMachine_h

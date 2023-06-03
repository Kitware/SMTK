//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_adaptor_ConfigureOperation_h
#define smtk_task_adaptor_ConfigureOperation_h

#include "smtk/task/Adaptor.h"

namespace smtk
{
namespace task
{
namespace adaptor
{

/// Configure a task with a resource and role given a dependent producer.
class SMTKCORE_EXPORT ConfigureOperation : public Adaptor
{
public:
  smtkTypeMacro(smtk::task::adaptor::ConfigureOperation);
  smtkSuperclassMacro(smtk::task::Adaptor);
  smtkCreateMacro(smtk::task::Adaptor);

  /// Construct an unconfigured adaptor.
  ConfigureOperation();
  ConfigureOperation(const Configuration& config);
  ConfigureOperation(const Configuration& config, Task* from, Task* to);

  /// Reconfigure the "to()" task. (required override)
  ///
  /// This method is called when the "from()" task changes into a
  /// completable state.
  bool reconfigureTask() override;

protected:
  void configureSelf(const Configuration& config);
};
} // namespace adaptor
} // namespace task
} // namespace smtk

#endif // smtk_task_adaptor_ConfigureOperation_h

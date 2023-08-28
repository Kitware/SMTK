//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_adaptor_ResourceAndRole_h
#define smtk_task_adaptor_ResourceAndRole_h

#include "smtk/task/Adaptor.h"

namespace smtk
{
namespace task
{
namespace adaptor
{

/// Configure a task with a resource and role given a dependent producer.
class SMTKCORE_EXPORT ResourceAndRole : public Adaptor
{
public:
  smtkTypeMacro(smtk::task::adaptor::ResourceAndRole);
  smtkSuperclassMacro(smtk::task::Adaptor);
  smtkCreateMacro(smtk::task::Adaptor);

  /// Construct an unconfigured adaptor.
  ResourceAndRole();
  ResourceAndRole(const Configuration& config);
  ResourceAndRole(const Configuration& config, Task* from, Task* to);

  /// Reconfigure the "to()" task.
  ///
  /// This method is called when the "from()" task changes state.
  bool updateDownstreamTask(State upstreamPrev, State upstreamNext) override;

protected:
  void configureSelf(const Configuration& config);

  std::string m_fromTag;
  std::string m_toTag;
};
} // namespace adaptor
} // namespace task
} // namespace smtk

#endif // smtk_task_adaptor_ResourceAndRole_h

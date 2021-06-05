//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Factory_h
#define smtk_task_Factory_h

#include "smtk/task/Task.h"

#include "smtk/common/Factory.h"
#include "smtk/common/TypeName.h"

#include <functional>
#include <map>
#include <string>

namespace smtk
{
namespace task
{

using ParentFactory = smtk::common::Factory<
  Task,
  void,
  smtk::common::factory::Inputs<Task::Configuration&, smtk::common::Managers::Ptr>,
  smtk::common::factory::Inputs<
    Task::Configuration&,
    smtk::task::Task::PassedDependencies,
    smtk::common::Managers::Ptr>>;
/**\brief A factory for tasks that compose a workflow.
  *
  * A task factory creates tasks for use in workflows.
  *
  * To register a Task subclass, it must have a default constructor
  * and constructors that take (1) configuration information,
  * (2) references to dependencies and (3) managers.
  *
  * This class overrides the base-class create methods with
  * versions that return shared pointers so that observers
  * of the factory can be passed shared (rather than unique)
  * pointers.
  */
class SMTKCORE_EXPORT Factory : public ParentFactory
{
public:
  using BaseType = smtk::task::Task;

  /// Create an instance of a Task.
  ///
  /// Unlike smtk::common::Factory, this returns a shared pointer.
  template<typename Type, typename... Args>
  std::shared_ptr<Type> create(Args&&... args) const
  {
    std::shared_ptr<BaseType> shared =
      this->ParentFactory::createFromIndex(typeid(Type).hash_code(), std::forward<Args>(args)...);
    return std::static_pointer_cast<Type>(shared);
  }

  /// Create an instance of a Task using its type name.
  ///
  /// Unlike smtk::common::Factory, this returns a shared pointer.
  template<typename... Args>
  std::shared_ptr<BaseType> createFromName(const std::string& typeName, Args&&... args) const
  {
    return this->ParentFactory::createFromName(typeName, std::forward<Args>(args)...);
  }

  /// Create an instance of a Task using its type name.
  ///
  /// Unlike smtk::common::Factory, this returns a shared pointer.
  template<typename... Args>
  std::shared_ptr<BaseType> createFromIndex(const std::size_t& typeIndex, Args&&... args) const
  {
    return this->ParentFactory::createFromIndex(typeIndex, std::forward<Args>(args)...);
  }
};
} // namespace task
} // namespace smtk

#endif

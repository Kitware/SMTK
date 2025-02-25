//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Worklet_h
#define smtk_task_Worklet_h

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/Managers.h"

#include "nlohmann/json.hpp"

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace smtk
{
namespace task
{

class Manager;
class Gallery;

/**\brief Worklet represents a set of tasks created to reuse a set of workflow logic in multiple workflows.
  */
class SMTKCORE_EXPORT Worklet : public smtk::resource::Component
{
  friend class Gallery;

public:
  smtkTypeMacro(smtk::task::Worklet);
  smtkSuperclassMacro(smtk::resource::Component);
  smtkCreateMacro(smtk::task::Worklet);

  /// Worklets are configured with arbitrary JSON objects, though this may change.
  using Configuration = nlohmann::json;

  Worklet();
  Worklet(
    const Configuration& config,
    Manager& taskManager,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);

  ~Worklet() override = default;

  /// A method called by all constructors passed Configuration information.
  /// Calling this outside of the constructor will reconfigure the worklet
  void configure(const Configuration& config, Manager& taskManager);

  /// Return the worklet's configuration.
  const Configuration& configuration() const { return m_configuration; }

  /// Set/get the worklet's unique identifier.
  const common::UUID& id() const override { return m_id; }
  bool setId(const common::UUID& newId) override;

  /// Return the worklet's name
  std::string name() const override { return m_name; }

  /// Return the resource that owns the worklet
  const std::shared_ptr<resource::Resource> resource() const override;

  /// Return the schema used by the worklet
  const std::string& schema() const { return m_schema; }

  /// Return the version of the worklet
  int version() const { return m_version; }

  /// Return the operation name to be used to instantiate the worklet
  /// If none is specified in the worklet's configuration, then
  /// smtk::task::EmplaceWorklet will be used.
  const std::string& operationName() const { return m_operationName; }

  /// Return the description of the worklet
  const std::string& description() const { return m_description; }

  ///@{
  ///\brief Set/Get the categories associated with this worklet
  ///
  /// These categories are used by category expressions associated with a task's agents
  /// to see if the tasks generated from the worklet would be appropriate children tasks,
  /// or with the task manager to see if the generated tasks would be appropriate top-level
  /// tasks.

  void setCategories(const std::set<std::string>& cats);
  const std::set<std::string>& categories() const { return m_categories; }
  ///@}

  /// Return the Task Manager managing the worklet
  std::shared_ptr<Manager> manager() const;

protected:
  /// Sets the name of the Worklet - to be used via the smtk::task::Gallery
  void setName(const std::string& newName);

  /// Worklet's name to present to the user.
  std::string m_name;
  /// Worklet's UUID.
  smtk::common::UUID m_id;
  /// Worklet's schema.
  std::string m_schema;
  /// Worklet's version.
  int m_version = 0;
  /// Worklet's operation name
  std::string m_operationName;
  /// Worklet's description string
  std::string m_description;
  /// If this worklet is being managed, this will refer to its manager.
  std::weak_ptr<smtk::task::Manager> m_manager;
  /// The JSON configuration of the worklet
  Configuration m_configuration;
  /// Categories associated with the worklet
  std::set<std::string> m_categories;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Worklet_h

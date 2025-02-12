//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Port_h
#define smtk_task_Port_h

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/task/State.h"

#include "nlohmann/json.hpp"

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

namespace smtk
{
namespace task
{

class Manager;
class PortData;
class Task;

/**\brief Ports are components that pass information between tasks of a workflow.
  *
  * Tasks are objects in a workflow that require users to perform work,
  * make a decision, or provide information. Ports provide a way to model
  * the flow of user-provided information between tasks.
  *
  * Each port provides types of information that will be passed to or from
  * its parent task.
  *
  * A task may own 0 or more ports. Workflow designers and/or users may
  * connect ports owned by different tasks via an adaptor; as long as the
  * ports model information of matching types, data can be fetched as needed
  * by the downstream task. When requesting data from a task, its ports are
  * used as keys.
  */
class SMTKCORE_EXPORT Port : public smtk::resource::Component
{
public:
  smtkTypeMacro(smtk::task::Port);
  smtkSuperclassMacro(smtk::resource::Component);
  smtkCreateMacro(smtk::resource::PersistentObject);

  /// Whether the task fetches data from a port or provides data for a port.
  enum Direction
  {
    In, //!< The task fetches data from this port.
    Out //!< The task provides data for this port.
  };

  /// Whether the task fetches data from a port or provides data for a port.
  enum Access
  {
    Internal, //!< This port is inward facing and is used by its task's children.
    External  //!< This port is outward facing and is used by other tasks
  };

  /// Ports may be configured with arbitrary JSON objects.
  using Configuration = nlohmann::json;

  Port();
  Port(
    const Configuration& config,
    Task* parentTask,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);
  Port(
    const Configuration& config,
    Manager& taskManager,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);

  /// A method called by all constructors passed Configuration information.
  ///
  /// In general, this method should set member variables directly
  /// instead of calling set/get methods since those may invoke observers
  /// with an uninitialized object.
  virtual void configure(const Configuration& config);

  /// Add port connections from configuration data.
  ///
  /// This method is called by configure() but may also be called
  /// elsewhere. Specifically, the task::Manager's `from_json()`
  /// function calls it after all of a project's persistent objects
  /// have been constructed to ensure the connections are able to
  /// be assigned.
  ///
  /// Note that \a connConfig is not the same as the \a config parameter
  /// passed to Port::configure(). Instead, it is a subitem of \a config
  /// that holds only connections.
  virtual void configureConnections(const Configuration& connConfig);

  /// Set/get the task's unique identifier.
  const common::UUID& id() const override { return m_id; }
  bool setId(const common::UUID& newId) override;

  /// Return the task's name
  std::string name() const override { return m_name; }
  virtual bool setName(const std::string& name);

  /// Return the port's direction.
  ///
  /// A port's direction may not change; it should be set by configure(), which
  /// is called at construction.
  Direction direction() const { return m_direction; }

  /// Return the port's access.
  ///
  /// A port's access may not change; it should be set by configure(), which
  /// is called at construction.
  Access access() const { return m_access; }

  /// Return the types of data this port is allowed to pass.
  std::unordered_set<smtk::string::Token>& dataTypes() { return m_dataTypes; }
  const std::unordered_set<smtk::string::Token>& dataTypes() const { return m_dataTypes; }

  /// Return the set of port connections.
  ///
  /// If the Port::Direction is In, these connections are
  /// objects upstream of the port. If the Port::Direction
  /// is Out, the connected objects are downstream of the port.
  const std::unordered_set<PersistentObject*>& connections() const { return m_connections; }
  std::unordered_set<PersistentObject*>& connections() { return m_connections; }

  /// Return PortData for a given object in connections().
  ///
  /// If \a object is *not* a port, the base Port class will either
  /// (a) construct an ObjectsInRoles instance holding the \a object in the unassignedRole(); or
  /// (b) return a null pointer, depending on whethern ObjectsInRoles
  /// is listed in the set returned by dataTypes().
  ///
  /// If \a object is a port, the task will be asked to produce a PortData
  /// instance.
  virtual std::shared_ptr<PortData> portData(smtk::resource::PersistentObject* object) const;

  const std::shared_ptr<resource::Resource> resource() const override;

  /// Return a role to be assigned to connections that are not ports.
  ///
  /// Since persistent objects that are not ports cannot produce smtk::task::PortData
  /// themselves (that includes roles for objects), this helps ports produce
  /// ObjectsInRoles data (where a role is required).
  smtk::string::Token unassignedRole() const { return m_unassignedRole; }

  /// Set/get style classes for the port.
  /// A style class specifies how applications should present the port
  /// (e.g., what type of view to provide the user, what rendering mode
  /// to use, what objects to list or exclude).
  const std::unordered_set<smtk::string::Token>& style() const { return m_style; }
  bool addStyle(const smtk::string::Token& styleClass);
  bool removeStyle(const smtk::string::Token& styleClass);
  bool clearStyle();

  /// Populate a type-container with view-related data for configuration.
  ///
  /// Subclasses should override this method.
  /// Generally, views will want access to a resource and potentially
  /// components in the resource that are the subject of the view.
  /// Other view configuration will come from view style() (see above)
  /// or smtk::common::Managers.
  ///
  /// This method will return true when the \a configuration
  /// was modified and false otherwise.
  virtual bool getViewData(smtk::common::TypeContainer& configuration) const;

  /// Return a parent task if one exists; null otherwise.
  Task* parent() const { return m_parent; }
  /// Set the parent task of a port.
  ///
  /// Tasks should call this when they take ownership of a port.
  bool setParent(Task* parent);

  /// Convert Direction enumerants to/from labels.
  static Direction DirectionFromLabel(const std::string& label);
  static smtk::string::Token LabelFromDirection(Direction dir);

  /// Convert Access enumerants to/from labels.
  static Access AccessFromLabel(const std::string& label);
  static smtk::string::Token LabelFromAccess(Access ac);

  /// Return an XHTML description of the port and its data.
  std::string describe() const;

protected:
  /// The unique identifier for this task.
  smtk::common::UUID m_id;
  /// A task name to present to the user.
  std::string m_name;
  /// The task which owns this port.
  Task* m_parent = nullptr;
  /// If this task is being managed, this will refer to its manager.
  std::weak_ptr<smtk::task::Manager> m_manager;
  /// The direction of this port.
  Direction m_direction{ Direction::In };
  /// The access of this port with respect to its task
  Access m_access{ Access::External };
  /// The port's upstream/downstream connections.
  std::unordered_set<PersistentObject*> m_connections;
  /// The subclasses of PortData this port is allowed to pass.
  std::unordered_set<smtk::string::Token> m_dataTypes;
  /// The role which "bare" objects in m_connections should be assigned.
  ///
  /// This defaults to "unassigned".
  smtk::string::Token m_unassignedRole;
  /// The set of style classes for this task.
  std::unordered_set<smtk::string::Token> m_style;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Port_h

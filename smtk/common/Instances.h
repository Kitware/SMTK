//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Instances_h
#define smtk_common_Instances_h

#include "smtk/common/Factory.h"
#include "smtk/common/Observers.h"
#include "smtk/common/Visit.h"

namespace smtk
{
namespace common
{

/// Events that an instance-manager can observe.
enum class InstanceEvent
{
  Managed,   //!< An instance of a managed class was added to the instance manager.
  Unmanaged, //!< An instance of a managed class was removed from the instance manager.
  Modified   //!< An instance has been marked modified (by an operation).
};

/// The Instances class is used to manage instances of objects which
/// share an inherited base type (the BaseType template parameter).
/// It owns references to the instances, which keeps them alive.
/// It provides methods to observe when instances are managed (added
/// to Instances) and unmanaged (removed from Instances — at which
/// point their destructor is usually called, although destruction
/// only occurs if no other shared pointers to the object exist).
///
/// Instances are created via smtk::common::Factory, which this class
/// inherits, and observers are invoked via smtk::common::Observers.
/// The Instances class mutates the signature of its parent Factory
/// class's creation methods so that they return shared pointers.
template<typename BaseType, typename... InputTypes>
class SMTK_ALWAYS_EXPORT Instances : public Factory<BaseType, InputTypes...>
{
public:
  /// An alias for the inherited parent class.
  using Superclass = smtk::common::Factory<BaseType, InputTypes...>;
  /// The signature of observers watching managed instance lifecycle events.
  using Observer = std::function<void(InstanceEvent, const std::shared_ptr<BaseType>&)>;
  /// Access to the set of observers of instances.
  using Observers = smtk::common::Observers<Observer>;
  /// The signature used to visit managed instances of objects.
  using Visitor = std::function<smtk::common::Visit(const std::shared_ptr<BaseType>&)>;

  /// Construct a manager of object instances.
  Instances()
    : m_observers([this](Observer& observer) {
      for (const auto& instance : m_instances)
      {
        observer(InstanceEvent::Managed, instance);
      }
    })
  {
  }

  /// Delete copy constructor and assignment operator.
  Instances(const Instances&) = delete;
  void operator=(const Instances&) = delete;

  /// Unregister a Type using its type name.
  using Superclass::unregisterType; // (const std::string&);

  /// Determine whether or not a Type is available using its type name.
  using Superclass::contains; // (const std::string&) const;

  /// Create an instance of the given \a Type and manage it.
  template<typename Type, typename... Args>
  std::shared_ptr<Type> create(Args&&... args)
  {
    std::shared_ptr<BaseType> instance = this->Superclass::createFromIndex(
      typeid(Type).hash_code(), std::forward<Args>(args)...); // .release();
    if (instance)
    {
      m_instances.insert(instance);
      m_observers(InstanceEvent::Managed, instance);
    }
    return std::static_pointer_cast<Type>(instance);
  }

  /// Create and manage an instance of \a Type using its type-name.
  template<typename... Args>
  std::shared_ptr<BaseType> createFromName(const std::string& typeName, Args&&... args)
  {
    std::shared_ptr<BaseType> instance =
      this->Superclass::createFromName(typeName, std::forward<Args>(args)...);
    if (instance)
    {
      m_instances.insert(instance);
      m_observers(InstanceEvent::Managed, instance);
    }
    return instance;
  }

  /// Create and manage an instance of a Type using its type-index.
  template<typename... Args>
  std::shared_ptr<BaseType> createFromIndex(const std::size_t& typeIndex, Args&&... args)
  {
    std::shared_ptr<BaseType> instance =
      this->Superclass::createFromIndex(typeIndex, std::forward<Args>(args)...);
    if (instance)
    {
      m_instances.insert(instance);
      m_observers(InstanceEvent::Managed, instance);
    }
    return instance;
  }

  /// Manage an already-created instance of a class.
  ///
  /// This returns true if the instance was added and
  /// false otherwise (which can occur if passed a null
  /// pointer or an already-managed instance).
  bool manage(const std::shared_ptr<BaseType>& instance)
  {
    if (!instance)
    {
      return false;
    }
    bool didManage = m_instances.insert(instance).second;
    if (didManage)
    {
      m_observers(InstanceEvent::Managed, instance);
    }
    return didManage;
  }

  /// Unmanage (drop the reference to) an \a instance.
  ///
  /// This may result in the destruction of \a instance
  /// but is not guaranteed to do so (i.e., when other
  /// shared-pointers to \a instance exist).
  bool unmanage(const std::shared_ptr<BaseType>& instance)
  {
    if (!instance)
    {
      return false;
    }
    auto it = m_instances.find(instance);
    if (it == m_instances.end())
    {
      return false;
    }
    m_instances.erase(it);
    m_observers(InstanceEvent::Unmanaged, instance);
    return true;
  }

  /// Determine whether \a instance is managed or not.
  bool contains(const std::shared_ptr<BaseType>& instance)
  {
    return m_instances.find(instance) != m_instances.end();
  }

  /// Unmanage all instances.
  void clear()
  {
    for (const auto& instance : m_instances)
    {
      m_observers(InstanceEvent::Unmanaged, instance);
    }
    m_instances.clear();
  }

  /// Return the set of observers of instances (so that you can insert an observer).
  Observers& observers() { return m_observers; }

  /// Iterate over the collection of instances, invoking a visitor on each.
  ///
  /// The return value indicates whether iteration was terminated early or not.
  smtk::common::Visit visit(Visitor visitor) const
  {
    for (const auto& instance : m_instances)
    {
      if (visitor(instance) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visit::Halt;
      }
    }
    return smtk::common::Visit::Continue;
  }

  /// Return the number of instances being managed.
  std::size_t size() const { return m_instances.size(); }

private:
  /// The container that owns managed instances.
  using Container = std::set<std::shared_ptr<BaseType>>;
  /// The container of instances.
  Container m_instances;
  /// Observers of changes to m_instances.
  Observers m_observers;
};
} // namespace common
} // namespace smtk

#endif

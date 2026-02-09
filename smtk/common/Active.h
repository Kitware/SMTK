//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_Active_h
#define smtk_common_Active_h

#include "smtk/SharedFromThis.h"
#include "smtk/common/Instances.h"
#include "smtk/common/Observers.h"

namespace smtk
{
namespace common
{

/// This object provides applications a way to change and observe an active
/// object from a set of objects which inherit shared_from_this.
///
/// When passed an Instances object at construction, only managed objects may be active.
/// This ensures that before an object is destroyed this object
/// can notify observers the active object is becoming inactive.
///
/// If not passed an Instances object at construction, any task may become active but
/// unmanaged objects might be deleted without any notification that the active
/// objects changed.
/// You are strongly encouraged to pass Instances to the constructor.

template<typename InstancesType, typename ObjectType = typename InstancesType::ObjectType>
class SMTK_ALWAYS_EXPORT Active
{
public:
  smtkTypeMacroBase(smtk::common::Active<InstancesType, ObjectType>);

  /// The signature for observers of the active task.
  /// Observers are passed the previously-active task and the soon-to-be-active task.
  ///
  /// Note that either task may be null (i.e., it is possible to have no active task).
  using Observer = std::function<void(ObjectType*, ObjectType*)>;
  /// The container for registered observers.
  using Observers = smtk::common::Observers<Observer>;

  /// Construct an active-task tracker.
  Active(InstancesType* instances = nullptr)
    : m_instances(instances)
  {
    if (m_instances)
    {
      m_instancesObserver = m_instances->observers().insert(
        [this](smtk::common::InstanceEvent event, const std::shared_ptr<ObjectType>& object) {
          if (
            event == smtk::common::InstanceEvent::Unmanaged && object && object == m_active.lock())
          {
            m_observers(object.get(), nullptr);
            m_active.reset();
          }
        },
        /* priority */ 0,
        /* initialize */ false,
        "Observe object deletion for Active object notification.");
    }
  }

  virtual ~Active() = default;

  /// Return the active object (or nullptr if no object is active).
  ObjectType* object() const { return m_active.lock().get(); }

  /// Change the active task (or abandon the currently-active task by passing nullptr).
  ///
  /// This method returns true if the active task changed and false otherwise.
  /// Passing a task that is not managed by \a instances passed to the constructor
  /// will always return false.
  /// Passing a task that is already active or unavailable will return false.
  bool switchTo(ObjectType* obj)
  {
    auto current = m_active.lock();
    if (current.get() == obj)
    { // Cannot switch from obj to obj
      return false;
    }
    if (!obj)
    {
      if (current)
      {
        m_active.reset();
        m_observers(current.get(), nullptr);
        return true;
      }
      // Cannot switch from null to null.
      return false;
    }
    auto sharedObj = std::static_pointer_cast<ObjectType>(obj->shared_from_this());
    if (m_instances && !m_instances->contains(sharedObj))
    {
      // Only objects managed by m_instances can be active if we have instances tracked.
      return false;
    }
    m_active = sharedObj;
    m_observers(current.get(), obj);
    return true;
  }

  /// Return the set of active-task observers (so you can insert yourself).
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

private:
  InstancesType* m_instances{ nullptr };
  typename InstancesType::Observers::Key m_instancesObserver;
  std::weak_ptr<ObjectType> m_active;
  Observers m_observers;
};
} // namespace common
} // namespace smtk

#endif // smtk_common_Active_h

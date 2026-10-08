//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_SharedActive_h
#define smtk_common_SharedActive_h

#include "smtk/SharedFromThis.h"
#include "smtk/common/Instances.h"
#include "smtk/common/Observers.h"

namespace smtk
{
namespace common
{

/**\brief This object provides applications a way to change and observe an active
  *       object of a type which inherits std::enable_shared_from_this.
  *
  * When an object becomes active, a shared pointer to it is held by this class,
  * which keeps it in memory. This may not be desired—especially if the ObjectType
  * inherits smtk::resource::PersistentObject—because it can keep a destructor from
  * being called at an intuitive time.
  */
template<typename ObjectType>
class SMTK_ALWAYS_EXPORT SharedActive
{
public:
  smtkTypeMacroBase(smtk::common::SharedActive<ObjectType>);

  /// The signature for observers of the active task.
  /// Observers are passed the previously-active task and the soon-to-be-active task.
  ///
  /// Note that either task may be null (i.e., it is possible to have no active task).
  using Observer =
    std::function<void(const std::shared_ptr<ObjectType>&, const std::shared_ptr<ObjectType>&)>;
  /// The container for registered observers.
  using Observers = smtk::common::Observers<Observer>;

  /// Construct an active-task tracker.
  SharedActive()
    : m_observers([this](Observer observer) -> void {
      std::shared_ptr<ObjectType> blank;
      observer(blank, m_active);
    })
  {
  }
  virtual ~SharedActive() = default;

  /// Return the active object (or nullptr if no object is active).
  std::shared_ptr<ObjectType> object() const { return m_active; }

  /// Change the active task (or abandon the currently-active task by passing nullptr).
  ///
  /// This method returns true if the active task changed and false otherwise.
  /// Passing a task that is not managed by \a instances passed to the constructor
  /// will always return false.
  /// Passing a task that is already active or unavailable will return false.
  bool switchTo(const std::shared_ptr<ObjectType>& obj)
  {
    auto current = m_active;
    if (current == obj)
    { // Cannot switch from obj to obj
      return false;
    }
    m_active = obj;
    m_observers(current, m_active);
    return true;
  }

  /// Return the set of active-task observers (so you can insert yourself).
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

private:
  std::shared_ptr<ObjectType> m_active;
  Observers m_observers;
};

} // namespace common
} // namespace smtk

#endif // smtk_common_SharedActive_h

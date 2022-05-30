//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Observers_h
#define smtk_common_Observers_h

#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <type_traits>
#include <utility>

#define ADD_OBSERVER(key, observers, observer, priority, initialize)                               \
  key = (observers)->insert(                                                                       \
    observer,                                                                                      \
    priority,                                                                                      \
    initialize,                                                                                    \
    std::string(__FILE__) + std::string(": ") + std::to_string(__LINE__))

namespace smtk
{
namespace common
{

/// An Observer is a functor that is called when certain actions are performed.
/// This pattern allows for the injection of algorithms in response to a group
/// of actions (e.g. Resource added/removed, Operation about to run/has run).
/// Observers added to the Observers instance can also be initialized (allowing
/// for a retroactive response to items currently under observation).
///
/// Observer functions can be assigned an integral priority when they are added.
/// Observer functions with a higher priority value will be executed before
/// those with a lower priority. Observer functions with the same priority value
/// are called in the order in which they were inserted.
///
/// In addition to adding and removing Observer functors to an Observers
/// instance, the execution logic of Observers can be overridden at runtime by
/// inserting an override functor into the Observers instance. This allows for
/// a run-time configurable type of polymorphism where consuming code can change
/// the behavior of the Observers class, allowing consuming code to redefine the
/// context in which Observer functors are executed. By default, the Observers
/// call operator iterates over and calls each Observer functor.
///
/// Variants exist for Observer functors with no return value, where the
/// Observer functors are simply called in sequence, and for Observer functions
/// with a binary return value, where the observer results are aggregated via a
/// bitwise OR operator.
///
/// When an Observer functor is added to an instance of Observers, a
/// non-copyable Key is returned. The key can be used to remove the Observer
/// functor, and the Observer functor is also scoped by the Key (when the Key
/// goes out of scope, the Observer functor is removed from the Observers
/// instance) by default. To decouple the Key's lifetime from that of the
/// Observer functor, use the Key's release() method.
template<typename Observer, bool DebugObservers = false, int DefaultPriority = 0>
class Observers
{
  friend class Key;

public:
  /// A value to indicate the relative order in which an observer should be
  /// called. Larger is higher priority.
  typedef int Priority;

  /// The default priority for observers inserted without an explicit priority.
  static constexpr Priority Default = DefaultPriority;
  static constexpr Priority defaultPriority() { return Default; }

  /// A convenience that returns the lowest priority representable for observers.
  static constexpr Priority lowestPriority() { return std::numeric_limits<Priority>::lowest(); }

private:
  /// A key by which an Observer can be accessed within the Observers instance.
  struct InternalKey : std::pair<int, int>
  {
    // By default, construct a null key.
    InternalKey()
      : std::pair<Priority, int>(std::numeric_limits<Priority>::lowest(), -1)
    {
    }
    InternalKey(int i, int j)
      : std::pair<int, int>(i, j)
    {
    }
    ~InternalKey() = default;

    bool assigned() const { return this->second != -1; }

    bool operator<(const InternalKey& rhs) const
    {
      return this->first == rhs.first ? this->second < rhs.second : rhs.first < this->first;
    }
  };

public:
  class Key final : InternalKey
  {
    friend class Observers;

  public:
    Key()
      : InternalKey()
      , m_observers(nullptr)
    {
    }

    Key(const Key&) = delete;
    Key& operator=(const Key&) = delete;

    Key(Key&& key) noexcept
      : InternalKey(std::move(key))
      , m_observers(std::move(key.m_observers))
    {
      if (m_observers)
      {
        std::unique_lock<std::mutex> lock(m_observers->m_mutex);
        m_observers->m_keys[*this] = this;
        // NOLINTNEXTLINE(bugprone-use-after-move): ???
        key.m_observers = nullptr;
      }
    }

    Key& operator=(Key&& key) noexcept
    {
      if (m_observers)
      {
        m_observers->erase(*this);
      }

      static_cast<InternalKey&>(*this) = std::move(static_cast<InternalKey&>(key));
      m_observers = key.m_observers;

      if (m_observers)
      {
        std::unique_lock<std::mutex> lock(m_observers->m_mutex);
        m_observers->m_keys[*this] = this;
        key.m_observers = nullptr;
      }
      return *this;
    }

    ~Key()
    {
      if (m_observers)
      {
        m_observers->erase(*this);
        m_observers = nullptr;
      }
    }

    bool operator==(const Key& rhs)
    {
      return this->first == rhs.first && this->second == rhs.second;
    }
    bool operator!=(const Key& rhs) { return !((*this) == rhs); }

    bool assigned() const { return this->InternalKey::assigned(); }

    void release()
    {
      if (m_observers)
      {
        std::unique_lock<std::mutex> lock(m_observers->m_mutex);
        m_observers->m_keys.erase(*this);
      }
      m_observers = nullptr;
    }

  private:
    Key(const InternalKey& key, Observers* observers)
      : InternalKey(key)
      , m_observers(observers)
    {
      if (m_observers)
      {
        m_observers->m_keys[key] = this;
      }
    }

    Observers* m_observers;
  };

  /// A functor to optionally initialize Observers as they are inserted into the
  /// Observers instance.
  typedef std::function<void(Observer&)> Initializer;

  Observers()
    : m_initializer()
  {
  }
  Observers(Initializer&& initializer)
    : m_initializer(initializer)
  {
  }

  ~Observers()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto& keypair : m_keys)
    {
      keypair.second->m_observers = nullptr;
    }
  }

  /// The call operator calls each of its Observer functors in sequence if there
  /// is no override functor defined. Otherwise, it calls the override functor.
  template<class... Types>
  auto operator()(Types&&... args) -> decltype(std::declval<Observer>()(args...))
  {
    return m_override ? m_override.operator()(std::forward<Types>(args)...)
                      : callObserversDirectly(std::forward<Types>(args)...);
  }

  /// For Observer functors that return an integral value, call all Observer
  /// functors and aggregate their output using a bitwise OR operator.
  template<class... Types>
  auto callObserversDirectly(Types&&... args) -> typename std::enable_if<
    std::is_integral<decltype(std::declval<Observer>()(args...))>::value,
    decltype(std::declval<Observer>()(args...))>::type
  {
    decltype(std::declval<Observer>()(args...)) result = 0;

    // Toggle m_observing flag to enable caching of requests to erase observers.
    m_observing = true;
    for (auto& entry : m_observers)
    {
      // During iteration, some observers may have requested the erasure of
      // other observers. Rather than directly remove them from the map and
      // invalidate the iteration loop, we cache these requests and prevent
      // these observers from being called (as though they were erased).
      if (m_toErase.find(entry.first) == m_toErase.end())
      {
        if (DebugObservers)
        {
          std::cerr << "Calling observer (" << entry.first.first << ", " << entry.first.second
                    << "): " << m_descriptions[entry.first] << std::endl;
        }
        if (entry.first.assigned())
        {
          result |= entry.second(std::forward<Types>(args)...);
        }
      }
      else if (DebugObservers)
      {
        std::cerr << "Skipping erased observer (" << entry.first.first << ", " << entry.first.second
                  << "): " << m_descriptions[entry.first] << std::endl;
      }
    }
    // Now that the iteration loop is complete, it is now safe to erase
    // observers again.
    m_observing = false;

    // Remove observers that were marked for erasure during observation.
    for (auto& toErase : m_toErase)
    {
      if (DebugObservers)
      {
        std::cerr << "Erasing observer (" << toErase.first << ", " << toErase.second
                  << "): " << m_descriptions[toErase] << std::endl;
      }
      erase(toErase);
    }
    m_toErase.clear();

    return result;
  }

  /// For Observer functors that do not return an integral value, simply call
  /// all Observer functors.
  template<class... Types>
  auto callObserversDirectly(Types&&... args) -> typename std::enable_if<
    !std::is_integral<decltype(std::declval<Observer>()(args...))>::value,
    decltype(std::declval<Observer>()(args...))>::type
  {
    // Toggle m_observing flag to enable caching of requests to erase observers.
    m_observing = true;
    for (auto& entry : m_observers)
    {
      // During iteration, some observers may have requested the erasure of
      // other observers. Rather than directly remove them from the map and
      // invalidate the iteration loop, we cache these requests and prevent
      // these observers from being called (as though they were erased).
      if (m_toErase.find(entry.first) == m_toErase.end())
      {
        if (DebugObservers)
        {
          std::cerr << "Calling observer (" << entry.first.first << ", " << entry.first.second
                    << "): " << m_descriptions[entry.first] << std::endl;
        }
        if (entry.first.assigned())
        {
          entry.second(std::forward<Types>(args)...);
        }
      }
      else if (DebugObservers)
      {
        std::cerr << "Skipping erased observer (" << entry.first.first << ", " << entry.first.second
                  << "): " << m_descriptions[entry.first] << std::endl;
      }
    }
    // Now that the iteration loop is complete, it is now safe to erase
    // observers again.
    m_observing = false;

    // Remove observers that were marked for erasure during observation.
    for (auto& toErase : m_toErase)
    {
      if (DebugObservers)
      {
        std::cerr << "Erasing observer (" << toErase.first << ", " << toErase.second
                  << "): " << m_descriptions[toErase] << std::endl;
      }
      erase(toErase);
    }
    m_toErase.clear();
  }

  /// Ask to receive notification (and possibly a chance to respond to) events.
  /// An integral priority value determines the relative order in which the
  /// observer is called with respect to other observer functions. Observer
  /// functions with a higher priority are called before those with a lower
  /// priority. If the Observers instance has an initializer and initialization
  /// is requested, the Observer functor is initialized (this is commonly a
  /// means to run the Observer functor retroactively on things already under
  /// observation). The return value is a handle that can be used to unregister
  /// the observer.
  Key insert(Observer fn, Priority priority, bool initialize, std::string description = "")
  {
    // An observer's handle id (the second value in its key) defines the order
    // in which the observer is called at a specific priority level. We
    // monotonically increase this value for each priority value we encounter.
    int handleId;
    if (m_observers.empty())
    {
      // If there are no observers, then this observer is the first of its
      // priority level.
      handleId = 0;
    }
    else
    {
      // Search for the the last observer (the one with the highest handle id)
      // at the requested priority. To do this, we first access the first
      // observer at the priority lower than the requested priority. We then
      // access the observer before it.
      typename std::map<InternalKey, Observer>::iterator upper;
      if (priority != std::numeric_limits<Priority>::lowest())
      {
        auto key = InternalKey(priority - 1, -1);
        upper = m_observers.upper_bound(key);

        // If the found observer is the first observer in the map, the incident
        // observer is the first at its priority level. It will be assigned as
        // such by the subsequent logic of this method without the iterator
        // decrement.
        if (upper != m_observers.begin())
        {
          --upper;
        }
      }
      else
      {
        // If the requested priority is unset, we return the
        // last observer in the map.
        upper = --m_observers.end();
      }
      // If the observer we found is at the same priority as the incident
      // observer, the new handle id is one higher than the found observer's
      // handle id. Otherwise, this is the first observer at this priority
      // level.
      handleId = (upper->first.first == priority ? upper->first.second + 1 : 0);
    }
    InternalKey handle = InternalKey(priority, handleId);
    if (initialize && m_initializer)
    {
      m_initializer(fn);
    }

    m_descriptions.insert(std::make_pair(handle, description));
    if (DebugObservers)
    {
      std::cerr << "Inserting observer (" << handle.first << ", " << handle.second
                << "): " << description << std::endl;
    }
    return m_observers.insert(std::make_pair(handle, fn)).second ? Key(handle, this) : Key();
  }

  Key insert(Observer fn, std::string description = "")
  {
    return insert(fn, DefaultPriority, true, description);
  }

  /// Indicate that an observer should no longer be called. Returns the number
  /// of remaining observers.
  std::size_t erase(Key& handle)
  {
    handle.release();

    if (m_observing)
    {
      if (DebugObservers)
      {
        std::cerr << "Queueing observer erasure (" << handle.first << ", " << handle.second
                  << "): " << m_descriptions[handle] << std::endl;
      }
      m_toErase.insert(handle);
      return m_observers.size() - m_toErase.size();
    }
    if (DebugObservers)
    {
      std::cerr << "Immediate observer erasure (" << handle.first << ", " << handle.second
                << "): " << m_descriptions[handle] << std::endl;
    }
    return erase(static_cast<InternalKey&>(handle));
  }

  /// Return the observer for the given key if one exists or nullptr otherwise.
  Observer find(const Key& handle) const
  {
    auto entry = m_observers.find(handle);
    return entry == m_observers.end() ? nullptr : entry->second;
  }

  /// Return the number of Observer functors in this instance.
  std::size_t size() const { return m_observers.size(); }

  /// Replace the default implementation (calling each Observer functor in
  /// sequence) with a new behavior.
  void overrideWith(Observer fn) { m_override = fn; }

  /// Remove the overriding behavior, restoring the default behavior (calling
  /// each Observer functor when Observers is called).
  void removeOverride() { m_override = Observer(); }

  const Initializer& initializer() const { return m_initializer; }

  void setInitializer(Initializer fn) { m_initializer = fn; }

  std::string description(Key handle) const { return m_descriptions[handle]; }

protected:
  // A map of observers. The observers are held in a map so that they can be
  // referenced (and therefore removed) at a later time using the observer's
  // associated key.
  std::map<InternalKey, Observer> m_observers;

  // A map of descriptions. A descriptions can be manually added to an observer
  // during its insertion, or it can automatically refer to the location of the
  // observer's insertion in the source code if the ADD_OBSERVER macro is used.
  std::map<InternalKey, std::string> m_descriptions;

  // A functor to override the default behavior of the Observers' call method.
  Observer m_override;

  // A functor to override the default initialize method.
  Initializer m_initializer;

private:
  std::size_t erase(const InternalKey& key)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_keys.erase(key);
    return m_observers.erase(key);
  }

  bool m_observing{ false };
  std::set<InternalKey> m_toErase;

  std::mutex m_mutex;
  std::map<InternalKey, Key*> m_keys;
};
} // namespace common
} // namespace smtk

#endif // smtk_common_Observers_h

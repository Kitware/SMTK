//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_Observers_h
#define __smtk_common_Observers_h

#include <functional>
#include <limits>
#include <map>
#include <type_traits>
#include <utility>

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
template <typename Observer>
class Observers
{
public:
  /// A value to indicate the relative order in which an observer should be
  /// called. Larger is higher priority.
  typedef int Priority;

  /// A key by which an Observer can be accessed within the Observers instance.
  struct Key : std::pair<int, int>
  {
    // By default, construct a null key.
    Key()
      : std::pair<Priority, int>(std::numeric_limits<Priority>::lowest(), -1)
    {
    }
    Key(int i, int j)
      : std::pair<int, int>(i, j)
    {
    }
    bool assigned() const { return this->second != -1; }

    bool operator<(const Key& rhs) const
    {
      return this->first == rhs.first ? this->second < rhs.second : rhs.first < this->first;
    }
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

  /// The call operator calls each of its Observer functors in sequence if there
  /// is no override functor defined. Otherwise, it calls the override functor.
  template <class... Types>
  auto operator()(Types&&... args) -> decltype(std::declval<Observer>()(args...))
  {
    return m_override ? m_override.operator()(std::forward<Types>(args)...)
                      : callObserversDirectly(std::forward<Types>(args)...);
  }

  /// For Observer functors that return an integral value, call all Observer
  /// functors and aggregate their output using a bitwise OR operator.
  template <class... Types>
  auto callObserversDirectly(Types&&... args) ->
    typename std::enable_if<std::is_integral<decltype(std::declval<Observer>()(args...))>::value,
      decltype(std::declval<Observer>()(args...))>::type
  {
    decltype(std::declval<Observer>()(args...)) result = 0;

    // This careful loop allows an observer to erase itself.
    typename std::map<Key, Observer>::iterator entry = m_observers.begin();
    typename std::map<Key, Observer>::iterator next;
    for (next = entry; entry != m_observers.end(); entry = next)
    {
      ++next;
      result |= entry->second(std::forward<Types>(args)...);
    }
    return result;
  }

  /// For Observer functors that do not return an integral value, simply call
  /// all Observer functors.
  template <class... Types>
  auto callObserversDirectly(Types&&... args) ->
    typename std::enable_if<!std::is_integral<decltype(std::declval<Observer>()(args...))>::value,
      decltype(std::declval<Observer>()(args...))>::type
  {
    // This careful loop allows an observer to erase itself.
    typename std::map<Key, Observer>::iterator entry = m_observers.begin();
    typename std::map<Key, Observer>::iterator next;
    for (next = entry; entry != m_observers.end(); entry = next)
    {
      ++next;
      entry->second(std::forward<Types>(args)...);
    }
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
  Key insert(Observer fn, Priority priority, bool initialize)
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
      typename std::map<Key, Observer>::iterator upper;
      if (priority != std::numeric_limits<Priority>::lowest())
      {
        auto key = Key(priority - 1, -1);
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
    Key handle = Key(priority, handleId);
    if (initialize && m_initializer)
    {
      m_initializer(fn);
    }
    return m_observers.insert(std::make_pair(handle, fn)).second ? handle : Key();
  }

  Key insert(Observer fn) { return insert(fn, std::numeric_limits<Priority>::lowest(), true); }

  /// Indicate that an observer should no longer be called. Returns the number
  /// of remaining observers.
  std::size_t erase(Key handle) { return m_observers.erase(handle); }

  /// Return the observer for the given key if one exists or nullptr otherwise.
  Observer find(Key handle) const
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

protected:
  // A map of observers. The observers are held in a map so that they can be
  // referenced (and therefore removed) at a later time using the observer's
  // associated key.
  std::map<Key, Observer> m_observers;

  // A functor to override the default behavior of the Observers' call method.
  Observer m_override;

  // A functor to override the default initialize method.
  Initializer m_initializer;
};
}
}

#endif // __smtk_common_Observers_h

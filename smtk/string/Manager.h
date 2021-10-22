//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_string_Manager_h
#define smtk_string_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h" // ignore MSVC warning C4251
#include "smtk/common/Observers.h"
#include "smtk/common/Visit.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace smtk
{
namespace string
{

/// A fixed-size integer type used to represent an arbitrary-length string.
using Hash = std::size_t;

/// The string manager class is a dictionary mapping integers to (constant) string values.
///
/// The manager also provides a way to store sets of strings (named with a string that is
/// itself hashed by the manager).
class SMTKCORE_EXPORT Manager
{
public:
  static std::shared_ptr<Manager> create();

  /// Events that can occur during the lifecycle of the manager.
  enum Event
  {
    Managed,  //!< A string was added to the manager.
    Inserted, //!< A string was inserted into a set.
    Removed,  //!< A string was removed from a set.
    Unmanaged //!< A string was removed from the manager.
  };
  /// Signature for functions visiting strings in the manager or in a set held by the manager.
  using Visitor = std::function<smtk::common::Visit(Hash entry)>;
  /// The signature used by observers to receive notifications from the string manager.
  ///
  /// The first parameter is the event type.
  /// The second parameter is the hash of the relevant string.
  /// The third parameter is the string itself.
  /// For Inserted/Removed events, the fourth parameter is the name of the set into which the hash is being inserted/removed.
  /// For other events, the fourth parameter is unspecified but you might generally expect it to be smtk::string::Manager::Invalid.
  using Observer = std::function<void(Event, Hash, const std::string&, Hash)>;
  /// The type of container that holds the set of smtk::string::Manager observers.
  using Observers = smtk::common::Observers<Observer>;
  /// An invalid hash (that should never exist inside the manager's storage).
  static constexpr Hash Invalid = 0;

  /// Insert a string into the manager by computing a unique hash (the returned value).
  Hash manage(const std::string& s);
  /// Remove a hash from the manager. This also removes it from any string sets.
  std::size_t unmanage(Hash h);

  /// Look up a string from its hashed value.
  const std::string& value(Hash h) const;
  /// Look up a hash from a string value (without inserting it).
  /// If the string has not been previously managed, then Manager::Invalid will be returned.
  Hash find(const std::string& s) const;
  /// Compute a hash from a string value (without inserting it into the manager).
  /// If the string is not already managed, this will compute the hash value
  /// that *would* be used if the string were to be immediately inserted.
  /// This method allows hash collisions to be avoided; one can compute a hash while the
  /// map is write-locked and insert if needed.
  ///
  /// Unlike the \a find() method, this will never return Manager::Invalid.
  Hash compute(const std::string& s) const;

  /// Add the hash \a h to the set \a s.
  ///
  /// The set \a s need not exist prior to this call.
  /// It will be added to the manager as needed and
  /// then used as a key in the dictionary of sets.
  /// The returned value is the hash of the set \a s (when passing a string for the set name)
  /// or a boolean indicating whether the insertion actually occurred (when passing a hash
  /// for the set name). Note that inserting an already-existing member will return false.
  Hash insert(const std::string& set, Hash h);
  bool insert(Hash set, Hash h);
  /// Remove the hash \a h from the set \a s.
  /// This returns true if the hash was removed and false otherwise (i.e., because
  /// the set did not exist or did not contain \a h.
  bool remove(const std::string& set, Hash h);
  bool remove(Hash set, Hash h);

  /// Return true if the \a set exists and contains hash \a h ; and false otherwise.
  ///
  /// If \a set is Invalid, then this returns true if the hash exists in \a m_data
  /// and false otherwise.
  bool contains(const std::string& set, Hash h) const;
  bool contains(Hash set, Hash h) const;

  /// Return true if the manager is empty (i.e., managing no hashes) and false otherwise.
  bool empty() const { return m_data.empty(); }

  /// Visit all members of the set (or the entire Manager if passed the Invalid hash).
  /// Your \a visitor may not modify the manager.
  /// You may terminate early by returning smtk::common::Halt.
  smtk::common::Visit visitMembers(Visitor visitor, Hash set = Invalid);
  /// Visit all set names in the manager.
  /// Your \a visitor may not modify the manager.
  /// You may terminate early by returning smtk::common::Halt.
  smtk::common::Visit visitSets(Visitor visitor);

  /// Return the set of string-manager observers (so you can insert/remove an observer).
  Observers& observers() { return m_observers; }

  /// Set the manager's storage. This exists to allow easy deserialization.
  void setData(
    const std::unordered_map<Hash, std::string>& members,
    const std::unordered_map<Hash, std::unordered_set<Hash>>& sets);

protected:
  /// Same as compute() but does not lock (you must hold m_writeLock upon entry).
  std::pair<Hash, bool> computeInternal(const std::string& s) const;
  std::pair<Hash, bool> computeInternalAndInsert(const std::string& s);

  Observers m_observers;
  std::unordered_map<Hash, std::string> m_data;
  std::unordered_map<Hash, std::unordered_set<Hash>> m_sets;
  mutable std::mutex m_writeLock;
};

/// A type-conversion operation to cast enumerants to strings.
SMTKCORE_EXPORT std::string eventName(const Manager::Event& e);
/// A type-conversion operation to cast strings to event types.
SMTKCORE_EXPORT Manager::Event eventEnum(const std::string& e);
/// Events may be appended to streams.
SMTKCORE_EXPORT std::ostream& operator<<(std::ostream& os, const Manager::Event& e);

} // namespace string
} // namespace smtk

#endif // smtk_string_Manager_h

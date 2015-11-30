//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_UUID_h
#define __smtk_common_UUID_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"
#include "smtk/HashFunctor.h"

#ifndef SHIBOKEN_SKIP
#include <boost/uuid/uuid.hpp>
#endif

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace smtk {
  namespace common {

//.NAME UUID - An RFC4122-compliant Universally Unique IDentifier (UUID)
//.SECTION Description
// This class uses Boost's UUID class to implement smtk::common::UUID.
// Unlike the Boost version, there is some shorthand to invoke the
// various generators.
class SMTKCORE_EXPORT UUID
{
public:
  typedef ::boost::uint8_t value_type;
  typedef ::boost::uint8_t* iterator;
  typedef ::boost::uint8_t const* const_iterator;

  typedef std::size_t size_type;

  UUID();
  UUID(const UUID& other);
  UUID(const_iterator begin, const_iterator end);
  UUID(const std::string& txt);
  UUID(const boost::uuids::uuid& data);

  /* Nice, but shiboken cannot handle:
  template<typename... Params>
  static std::vector<UUID> Array(Params... parameters)
    {
    std::vector<UUID> result;
    UUID::ConstructArray(result, parameters...);
    return result;
    }
    */

  static UUID random();
  static UUID null();

  enum {
    SIZE = 16
  };
  static size_type size() { return SIZE; }
  bool isNull() const;

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  std::string toString() const;

  bool operator != (UUID const& other) const;
  bool operator == (UUID const& other) const;
  bool operator < (UUID const& other) const;

  UUID& operator = (UUID const& other);

  operator bool() const;

  std::size_t hash() const;

protected:
  // Implemented using Boost's UUID library.
  boost::uuids::uuid m_data;

  /* Nice, but shiboken cannot handle:
  template<typename T, typename... Params>
  static T& ConstructArray(T& array)
    {
    return array;
    }

  template<typename T, typename... Params>
  static T& Array(T& array, const UUID& uid, Params... parameters)
    {
    array.push_back(uid);
    UUID::ConstructArray(array, parameters...);
    return array;
    }
    */
};

typedef std::set<UUID> UUIDs;
typedef std::vector<UUID> UUIDArray;
typedef std::vector<UUIDArray> UUIDArrays;

SMTKCORE_EXPORT std::ostream& operator << (std::ostream& stream, const UUID& uid);
SMTKCORE_EXPORT std::istream& operator >> (std::istream& stream, UUID& uid);

  } // namespace common
} // namespace smtk

SMTK_HASH_NS_BEGIN

#if SMTK_HASH_SPECIALIZATION == 1
// Specialize hash<UUID> functor
template <>
struct hash<smtk::common::UUID>
{
  size_t operator()(const smtk::common::UUID& uid) const
    {
    // Use the last sizeof(size_t) bytes as the hash since UUIDs
    // put their version number in the 4 LSBs of the 8th byte, which
    // causes collisions when sizeof(size_t) == 8.
    // This will need to be revisited if we switch to node-based
    // UUIDs, but for random UUIDs it works well.
    return *reinterpret_cast<const size_t*>(uid.begin() + uid.size() - sizeof(size_t));
    }
};
#else
// Specialize hash typecast operators
template <>
inline size_t
hash<const smtk::common::UUID&>::operator()(const smtk::common::UUID& uid) const
{
  // Use the last sizeof(size_t) bytes as the hash since UUIDs
  // put their version number in the 4 LSBs of the 8th byte, which
  // causes collisions when sizeof(size_t) == 8.
  // This will need to be revisited if we switch to node-based
  // UUIDs, but for random UUIDs it works well.
  return *reinterpret_cast<const size_t*>(uid.begin() + uid.size() - sizeof(size_t));
}

template <>
inline size_t
hash<smtk::common::UUID>::operator()(smtk::common::UUID uid) const
{
  // Use the last sizeof(size_t) bytes as the hash since UUIDs
  // put their version number in the 4 LSBs of the 8th byte, which
  // causes collisions when sizeof(size_t) == 8.
  // This will need to be revisited if we switch to node-based
  // UUIDs, but for random UUIDs it works well.
  return *reinterpret_cast<const size_t*>(uid.begin() + uid.size() - sizeof(size_t));
}
#endif // SMTK_HASH_SPECIALIZATION

SMTK_HASH_NS_END

#endif // __smtk_common_UUID_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_UUID_h
#define smtk_common_UUID_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <cstring>
#include <iosfwd>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace common
{

//.NAME UUID - An RFC4122-compliant Universally Unique IDentifier (UUID)
//.SECTION Description
// This class uses Boost's UUID class to implement smtk::common::UUID.
// Unlike the Boost version, there is some shorthand to invoke the
// various generators.
class SMTKCORE_EXPORT UUID
{
public:
  static constexpr const char* const type_name = "uuid";
  typedef ::boost::uint8_t value_type;
  typedef ::boost::uint8_t* iterator;
  typedef ::boost::uint8_t const* const_iterator;

  typedef std::size_t size_type;

  UUID();
  UUID(const UUID& other);
  UUID(const_iterator begin, const_iterator end);
  UUID(const std::string& txt);
  UUID(const boost::uuids::uuid& data);

  static UUID random();
  static UUID null();

  enum
  {
    SIZE = 16
  };
  static size_type size() { return SIZE; }
  bool isNull() const;

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  std::string toString() const;

  bool operator!=(UUID const& other) const;
  bool operator==(UUID const& other) const;
  bool operator<(UUID const& other) const;

  UUID& operator=(UUID const& other);

  operator bool() const;

  std::size_t hash() const;

protected:
  // Implemented using Boost's UUID library.
  boost::uuids::uuid m_data;
};

typedef std::set<UUID> UUIDs;
typedef std::vector<UUID> UUIDArray;
typedef std::vector<UUIDArray> UUIDArrays;

SMTKCORE_EXPORT std::ostream& operator<<(std::ostream& stream, const UUID& uid);
SMTKCORE_EXPORT std::istream& operator>>(std::istream& stream, UUID& uid);

} // namespace common
} // namespace smtk

namespace std
{

// Specialize std::hash<UUID> functor
template<>
struct hash<smtk::common::UUID>
{
  size_t operator()(const smtk::common::UUID& uid) const
  {
    size_t hash;
    // Use the last sizeof(size_t) bytes as the hash since UUIDs
    // put their version number in the 4 LSBs of the 8th byte, which
    // causes collisions when sizeof(size_t) == 8.
    // This will need to be revisited if we switch to node-based
    // UUIDs, but for random UUIDs it works well.
    memmove(&hash, uid.begin() + smtk::common::UUID::size() - sizeof(hash), sizeof(hash));
    return hash;
  }
};

} // namespace std

#endif // smtk_common_UUID_h

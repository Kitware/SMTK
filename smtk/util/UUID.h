#ifndef __smtk_util_UUID_h
#define __smtk_util_UUID_h

#include "smtk/SMTKCoreExports.h"

#include <boost/uuid/uuid.hpp>

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace smtk {
  namespace util {

//.NAME UUID - An RFC4122-compliant Universally Unique IDentifier (UUID)
//.SECTION Description
// This class uses Boost's UUID class to implement smtk::util::UUID.
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

  static size_type size() { return 16; }
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

  } // namespace util
} // namespace smtk

// Specialize std::hash<UUID>
namespace std {

template <>
struct hash<smtk::util::UUID>
{
  size_t operator()(const smtk::util::UUID& uid) const
    {
    return *reinterpret_cast<const size_t*>(uid.begin());
    /* your code here, e.g. "return hash<int>()(x.value);" */
    }
};

} // namespace std

#endif // __smtk_util_UUID_h

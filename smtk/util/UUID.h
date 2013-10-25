#ifndef __smtk_util_UUID_h
#define __smtk_util_UUID_h

#include "smtk/SMTKCoreExports.h"

#include <boost/uuid/uuid.hpp>

#include <string>
#include <iostream>

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
  typedef uint8_t value_type;
  typedef uint8_t* iterator;
  typedef uint8_t const* const_iterator;
  typedef std::size_t size_type;

  UUID();
  UUID(const UUID& other);
  UUID(const_iterator begin, const_iterator end);
  UUID(const std::string& txt);
  UUID(const boost::uuids::uuid& data);

  static UUID Random();
  static UUID Null();

  static size_type Size() { return 16; }
  bool IsNull() const;

  iterator Begin();
  const_iterator Begin() const;
  iterator End();
  const_iterator End() const;

  std::string ToString() const;

  bool operator != (UUID const& other) const;
  bool operator == (UUID const& other) const;
  bool operator < (UUID const& other) const;

  UUID& operator = (UUID const& other);

protected:
  // Implemented using Boost's UUID library.
  boost::uuids::uuid Data;
};

std::ostream& operator << (std::ostream& stream, const UUID& uid);
std::istream& operator >> (std::istream& stream, UUID& uid);

  } // namespace util
} // namespace smtk

#endif // __smtk_util_UUID_h

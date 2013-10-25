#include "smtk/util/UUID.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace smtk {
  namespace util {

/// Default constructor creates a nil UUID (IsNull() == true).
UUID::UUID()
 : Data()
{
}

/// Copy constructor.
UUID::UUID(const UUID& other)
{
  this->Data = other.Data;
}

/// Construct a UUID from raw bytes (more than twice as efficient as string conversion).
UUID::UUID(const_iterator begin, const_iterator end)
{
  for (iterator it = this->Data.begin(); begin != end; ++it, ++begin)
    {
    *it = *begin;
    }
}

/// Construct a UUID from a text string (36 characters long, including hyphens).
UUID::UUID(const std::string& txt)
{
  boost::uuids::string_generator sgen;
  this->Data = sgen(txt);
}

/// Construct a UUID from a boost UUID object.
UUID::UUID(const boost::uuids::uuid& data)
{
  this->Data = data;
}

/// Generate a random UUID (RFC4122, version 4)
UUID UUID::Random()
{
  boost::uuids::basic_random_generator<boost::mt19937> gen;
  return UUID(gen());
}

/// Generate a nil UUID.
UUID UUID::Null()
{
  boost::uuids::nil_generator ngen;
  return UUID(ngen());
}

/// Test whether a UUID is nil (i.e., all zeros) or not. The nil UUID usually indicates an invalid UUID.
bool UUID::IsNull() const
{
  return this->Data.is_nil();
}

/// Return an iterator to the start of a UUID's raw data.
UUID::iterator UUID::Begin()
{
  return this->Data.begin();
}

/// Return a constant iterator to the start of a UUID's raw data.
UUID::const_iterator UUID::Begin() const
{
  return this->Data.begin();
}

/// Return an iterator to the end of a UUID's raw data.
UUID::iterator UUID::End()
{
  return this->Data.end();
}

/// Return a constant iterator to the end of a UUID's raw data.
UUID::const_iterator UUID::End() const
{
  return this->Data.end();
}

/// Convert the UUID to a string.
std::string UUID::ToString() const
{
  return boost::uuids::to_string(this->Data);
}

/// Compare two UUIDs for inequality.
bool UUID::operator != (const UUID& other) const
{
  return this->Data != other.Data;
}

/// Compare two UUIDs for equality.
bool UUID::operator == (const UUID& other) const
{
  return this->Data == other.Data;
}

/// Compare two UUIDs for ordering.
bool UUID::operator < (const UUID& other) const
{
  return this->Data < other.Data;
}

/// Assignment operator.
UUID& UUID::operator = (const UUID& other)
{
  this->Data = other.Data;
  return *this;
}

/// Write a UUID to a stream (as a string).
std::ostream& operator << (std::ostream& stream, const UUID& uid)
{
  stream << uid.ToString().c_str();
  return stream;
}

/// Read a UUID from a stream (as a string).
std::istream& operator >> (std::istream& stream, UUID& uid)
{
  std::string txt;
  stream >> txt;
  uid = UUID(txt);
  return stream;
}

  } // namespace util
} // namespace smtk

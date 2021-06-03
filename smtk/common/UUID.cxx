//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iostream>

SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace common
{

/// Default constructor creates a nil UUID (IsNull() == true).
UUID::UUID()
  : m_data()
{
}

/// Copy constructor.
UUID::UUID(const UUID& other)
{
  m_data = other.m_data;
}

/// Construct a UUID from raw bytes (more than twice as efficient as string conversion).
UUID::UUID(const_iterator inBegin, const_iterator inEnd)
{
  for (iterator it = m_data.begin(); inBegin != inEnd; ++it, ++inBegin)
  {
    *it = *inBegin;
  }
}

/// Construct a UUID from a text string (36 characters long, including hyphens).
/// The UUID stays in its default state of nil if the input string is empty.
UUID::UUID(const std::string& txt)
  : m_data()
{
  if (!txt.empty())
  {
    boost::uuids::string_generator sgen;
    m_data = sgen(txt);
  }
}

/// Construct a UUID from a boost UUID object.
UUID::UUID(const boost::uuids::uuid& data)
{
  m_data = data;
}

/**\brief Generate a random UUID (RFC4122, version 4)
  *
  * This uses a thread-local UUIDGenerator instance to
  * avoid reinitializing a random-number generator each call.
  */
UUID UUID::random()
{
  auto& generator = UUIDGenerator::instance();
  return generator.random();
}

/// Generate a nil UUID.
UUID UUID::null()
{
  boost::uuids::nil_generator ngen;
  return UUID(ngen());
}

/// Test whether a UUID is nil (i.e., all zeros) or not. The nil UUID usually indicates an invalid UUID.
bool UUID::isNull() const
{
  return m_data.is_nil();
}

/// Return an iterator to the start of a UUID's raw data.
UUID::iterator UUID::begin()
{
  return m_data.begin();
}

/// Return a constant iterator to the start of a UUID's raw data.
UUID::const_iterator UUID::begin() const
{
  return m_data.begin();
}

/// Return an iterator to the end of a UUID's raw data.
UUID::iterator UUID::end()
{
  return m_data.end();
}

/// Return a constant iterator to the end of a UUID's raw data.
UUID::const_iterator UUID::end() const
{
  return m_data.end();
}

/// Convert the UUID to a string.
std::string UUID::toString() const
{
  return boost::uuids::to_string(m_data);
}

/// Compare two UUIDs for inequality.
bool UUID::operator!=(const UUID& other) const
{
  return m_data != other.m_data;
}

/// Compare two UUIDs for equality.
bool UUID::operator==(const UUID& other) const
{
  return m_data == other.m_data;
}

/// Compare two UUIDs for ordering.
bool UUID::operator<(const UUID& other) const
{
  return m_data < other.m_data;
}

/// Assignment operator.
UUID& UUID::operator=(const UUID& other) = default;

/// Cast-to-boolean operator
UUID::operator bool() const
{
  return !this->isNull();
}

/// Return a hash of a UUID.
std::size_t UUID::hash() const
{
  boost::hash<boost::uuids::uuid> uuid_hasher;
  return uuid_hasher(m_data);
}

/// Write a UUID to a stream (as a string).
std::ostream& operator<<(std::ostream& stream, const UUID& uid)
{
  stream << uid.toString().c_str();
  return stream;
}

/// Read a UUID from a stream (as a string).
std::istream& operator>>(std::istream& stream, UUID& uid)
{
  std::string txt;
  stream >> txt;
  uid = UUID(txt);
  return stream;
}

} // namespace common
} // namespace smtk

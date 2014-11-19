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

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored"-Wshadow"
#endif
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif

namespace smtk {
  namespace common {

/// Default constructor creates a nil UUID (IsNull() == true).
UUID::UUID()
 : m_data()
{
}

/// Copy constructor.
UUID::UUID(const UUID& other)
{
  this->m_data = other.m_data;
}

/// Construct a UUID from raw bytes (more than twice as efficient as string conversion).
UUID::UUID(const_iterator inBegin, const_iterator inEnd)
{
  for (iterator it = this->m_data.begin(); inBegin != inEnd; ++it, ++inBegin)
    {
    *it = *inBegin;
    }
}

/// Construct a UUID from a text string (36 characters long, including hyphens).
UUID::UUID(const std::string& txt)
{
  boost::uuids::string_generator sgen;
  this->m_data = sgen(txt);
}

/// Construct a UUID from a boost UUID object.
UUID::UUID(const boost::uuids::uuid& data)
{
  this->m_data = data;
}

/**\brief Generate a random UUID (RFC4122, version 4)
  *
  * WARNING: This method will create a new basic_random_generator
  * instance, which is very slow to construct.
  */
UUID UUID::random()
{
  boost::uuids::basic_random_generator<boost::mt19937> gen;
  return UUID(gen());
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
  return this->m_data.is_nil();
}

/// Return an iterator to the start of a UUID's raw data.
UUID::iterator UUID::begin()
{
  return this->m_data.begin();
}

/// Return a constant iterator to the start of a UUID's raw data.
UUID::const_iterator UUID::begin() const
{
  return this->m_data.begin();
}

/// Return an iterator to the end of a UUID's raw data.
UUID::iterator UUID::end()
{
  return this->m_data.end();
}

/// Return a constant iterator to the end of a UUID's raw data.
UUID::const_iterator UUID::end() const
{
  return this->m_data.end();
}

/// Convert the UUID to a string.
std::string UUID::toString() const
{
  return boost::uuids::to_string(this->m_data);
}

/// Compare two UUIDs for inequality.
bool UUID::operator != (const UUID& other) const
{
  return this->m_data != other.m_data;
}

/// Compare two UUIDs for equality.
bool UUID::operator == (const UUID& other) const
{
  return this->m_data == other.m_data;
}

/// Compare two UUIDs for ordering.
bool UUID::operator < (const UUID& other) const
{
  return this->m_data < other.m_data;
}

/// Assignment operator.
UUID& UUID::operator = (const UUID& other)
{
  this->m_data = other.m_data;
  return *this;
}

/// Cast-to-boolean operator
UUID::operator bool () const
{
  return this->isNull() ? false : true;
}

/// Return a hash of a UUID.
std::size_t UUID::hash() const
{
  boost::hash<boost::uuids::uuid> uuid_hasher;
  return uuid_hasher(this->m_data);
}

/// Write a UUID to a stream (as a string).
std::ostream& operator << (std::ostream& stream, const UUID& uid)
{
  stream << uid.toString().c_str();
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

  } // namespace common
} // namespace smtk

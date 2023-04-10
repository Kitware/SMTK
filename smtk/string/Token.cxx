//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/string/Token.h"

#include <cstring>
#include <exception>
#include <thread>

static std::mutex s_managerLock;

namespace smtk
{
namespace string
{

std::shared_ptr<Manager> Token::s_manager;

Token::Token(const char* data, std::size_t size)
{
  if (!data)
  {
    m_id = Manager::Invalid;
  }
  else
  {
    if (size == std::string::npos)
    {
      size = std::strlen(data);
    }
    m_id = Token::manager().manage(std::string(data, size));
  }
}

Token::Token(const std::string& data)
{
  m_id = Token::manager().manage(data);
}

const std::string& Token::data() const
{
  return Token::manager().value(m_id);
}

bool Token::hasData() const
{
  return Token::manager().hasValue(m_id);
}

bool Token::valid() const
{
  return m_id != smtk::string::Manager::Invalid;
}

bool Token::operator==(const Token& other) const
{
  return m_id == other.m_id;
}

bool Token::operator!=(const Token& other) const
{
  return m_id != other.m_id;
}

bool Token::operator<(const Token& other) const
{
  return this->data() < other.data();
}

bool Token::operator>(const Token& other) const
{
  return this->data() > other.data();
}

bool Token::operator<=(const Token& other) const
{
  return this->data() <= other.data();
}

bool Token::operator>=(const Token& other) const
{
  return this->data() >= other.data();
}

Manager& Token::manager()
{
  if (s_manager)
  {
    return *s_manager;
  }
  else
  {
    std::lock_guard<std::mutex> lock(s_managerLock);
    s_manager = Manager::create();
    return *s_manager;
  }
}

Token Token::fromHash(Hash h)
{
  Token result;
  if (!s_manager->verify(result.m_id, h))
  {
    throw std::invalid_argument("Hash does not exist in database.");
  }
  return result;
}

} // namespace string
} // namespace smtk

bool operator==(const std::string& a, const smtk::string::Token& b)
{
  return a == b.data();
}
bool operator!=(const std::string& a, const smtk::string::Token& b)
{
  return a != b.data();
}
bool operator>(const std::string& a, const smtk::string::Token& b)
{
  return a > b.data();
}
bool operator<(const std::string& a, const smtk::string::Token& b)
{
  return a < b.data();
}
bool operator>=(const std::string& a, const smtk::string::Token& b)
{
  return a >= b.data();
}
bool operator<=(const std::string& a, const smtk::string::Token& b)
{
  return a <= b.data();
}

bool operator==(const smtk::string::Token& a, const std::string& b)
{
  return a.data() == b;
}
bool operator!=(const smtk::string::Token& a, const std::string& b)
{
  return a.data() != b;
}
bool operator>(const smtk::string::Token& a, const std::string& b)
{
  return a.data() > b;
}
bool operator<(const smtk::string::Token& a, const std::string& b)
{
  return a.data() < b;
}
bool operator>=(const smtk::string::Token& a, const std::string& b)
{
  return a.data() >= b;
}
bool operator<=(const smtk::string::Token& a, const std::string& b)
{
  return a.data() <= b;
}

bool operator==(const char* a, const smtk::string::Token& b)
{
  return std::string(a) == b.data();
}
bool operator!=(const char* a, const smtk::string::Token& b)
{
  return std::string(a) != b.data();
}
bool operator>(const char* a, const smtk::string::Token& b)
{
  return std::string(a) > b.data();
}
bool operator<(const char* a, const smtk::string::Token& b)
{
  return std::string(a) < b.data();
}
bool operator>=(const char* a, const smtk::string::Token& b)
{
  return std::string(a) >= b.data();
}
bool operator<=(const char* a, const smtk::string::Token& b)
{
  return std::string(a) <= b.data();
}

bool operator==(const smtk::string::Token& a, const char* b)
{
  return a.data() == std::string(b);
}
bool operator!=(const smtk::string::Token& a, const char* b)
{
  return a.data() != std::string(b);
}
bool operator>(const smtk::string::Token& a, const char* b)
{
  return a.data() > std::string(b);
}
bool operator<(const smtk::string::Token& a, const char* b)
{
  return a.data() < std::string(b);
}
bool operator>=(const smtk::string::Token& a, const char* b)
{
  return a.data() >= std::string(b);
}
bool operator<=(const smtk::string::Token& a, const char* b)
{
  return a.data() <= std::string(b);
}

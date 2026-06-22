//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_db_Query_h
#define smtk_job_db_Query_h

#include "smtk/job/db/Binding.h"

#include "sqlite3.h"

namespace smtk
{
namespace job
{
namespace db
{

class SMTKCORE_EXPORT sqlQuery
{
public:
  sqlQuery(sqlite3* db, const std::string& statement = std::string())
    : m_db(db)
  {
    if (!statement.empty())
    {
      m_query << statement << '\0';
      const char* tail;
      if (
        sqlite3_prepare(
          m_db, m_query.str().c_str(), static_cast<int>(m_query.str().size()), &m_cursor, &tail) !=
        SQLITE_OK)
      {
        throw std::logic_error("Could not prepare statement.");
      }
    }
  }

  ~sqlQuery()
  {
    if (m_cursor)
    {
      sqlite3_finalize(m_cursor);
      m_cursor = nullptr;
    }
  }

  std::string str() { return m_query.str(); }

  bool execute()
  {
    if (!m_db)
    {
      return false;
    }

    if (!m_cursor)
    {
      if (m_query.str().empty())
      {
        return false;
      }
      const char* tail;
      if (
        sqlite3_prepare(
          m_db, m_query.str().c_str(), static_cast<int>(m_query.str().size()), &m_cursor, &tail) !=
        SQLITE_OK)
      {
        return false;
      }
      m_query.clear();
      m_query.seekp(0);
    }

    int step;
    for (step = sqlite3_step(m_cursor); step == SQLITE_ROW; step = sqlite3_step(m_cursor))
    {
      for (auto& binding : m_bindings)
      {
        if (!binding->captureValue(*this))
        {
          return false;
        }
      }
    }
    if (step != SQLITE_DONE)
    {
      std::cerr << "Failed to execute query. (code: " << step << ", query: " << m_query.str()
                << ")\n";
    }
    return step == SQLITE_DONE;
  }

  sqlite3_stmt* cursor() const { return m_cursor; }

  template<typename ValueType>
  sqlQuery& operator<<(const ValueType& value)
  {
    // Modifying the query causes finalization of any prior prepared statement.
    if (m_cursor)
    {
      m_bindings.clear();
      sqlite3_finalize(m_cursor);
      m_cursor = nullptr;
      m_query.clear();
      m_query.str("");
      m_query.seekp(0);
    }
    m_query << value;
    return *this;
  }

  template<typename BinderType>
  sqlQuery& bind(std::size_t index, const typename BinderType::container_type& container)
  {
    m_bindings.push_back(std::make_shared<BinderType>(index, container));
    return *this;
  }

  template<typename ContainerType>
  sqlQuery& bindText(std::size_t index, const ContainerType& container)
  {
    m_bindings.push_back(
      std::make_shared<sqlBindingText<typename ContainerType::value_type, ContainerType>>(
        index, container));
    return *this;
  }

  template<typename ContainerType>
  sqlQuery& bindInt(std::size_t index, const ContainerType& container)
  {
    m_bindings.push_back(
      std::make_shared<sqlBindingInt<typename ContainerType::value_type, ContainerType>>(
        index, container));
    return *this;
  }

protected:
  std::ostringstream m_query;
  sqlite3_stmt* m_cursor{ nullptr };
  sqlite3* m_db{ nullptr };
  std::vector<std::shared_ptr<sqlBinding>> m_bindings;
};

} // namespace db
} // namespace job
} // namespace smtk

#endif // smtk_job_db_Query_h

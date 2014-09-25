//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/io/Logger.h"

namespace smtk {
  namespace io {

//----------------------------------------------------------------------------
void Logger::addRecord(Severity s, const std::string &m,
                       const std::string &fname,
                       unsigned int line)
{
  if ((s == Logger::ERROR) || (s == Logger::FATAL))
    {
    this->m_hasErrors = true;
    }
  this->m_records.push_back(Record(s, m, fname, line));
}
//----------------------------------------------------------------------------
void Logger::append(const Logger &l)
{
  this->m_records.insert(this->m_records.end(), l.m_records.begin(),
                         l.m_records.end());
  if (l.m_hasErrors)
    {
    this->m_hasErrors = true;
    }
}
//----------------------------------------------------------------------------
void Logger::reset()
{
  this->m_hasErrors = false;
  this->m_records.empty();
}
//----------------------------------------------------------------------------
std::string Logger::severityAsString(Severity s)
{
  switch (s)
    {
    case DEBUG:
      return "DEBUG";
    case INFO:
      return "INFO";
    case WARNING:
      return "WARNING";
    case ERROR:
      return "ERROR";
    case FATAL:
      return "FATAL";
    default:
      return "UNKNOWN";
    }
  return "UNKNOWN";
}
//----------------------------------------------------------------------------
std::string Logger::convertToString() const
{
  std::size_t i, n = this->m_records.size();
  std::stringstream ss;
  for (i = 0; i < n; i++)
    {
    ss << severityAsString(this->m_records[i].severity) << ": ";
    if (this->m_records[i].fileName != "")
      {
      ss << "In " << this->m_records[i].fileName << ", line "
         << this->m_records[i].lineNumber << ": ";
      }
    ss << this->m_records[i].message << std::endl;
    }
  return ss.str();
}

  } // namespace io
} // namespace smtk

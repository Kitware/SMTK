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
Logger::~Logger()
{
  this->setFlushToStream(NULL, false);
}

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
  if (this->m_file)
    {
    (*this->m_file) << this->toString(this->numberOfRecords() - 1);
    this->m_file->flush();
    }
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
  if (this->m_file)
    {
    (*this->m_file) << this->toString(
      this->numberOfRecords() - l.numberOfRecords(),
      this->numberOfRecords());
    this->m_file->flush();
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

/**\brief Convert the given log entry to a string.
  *
  */
std::string Logger::toString(std::size_t i) const
{
  std::stringstream ss;
  ss << severityAsString(this->m_records[i].severity) << ": ";
  if (this->m_records[i].fileName != "")
    {
    ss << "In " << this->m_records[i].fileName << ", line "
      << this->m_records[i].lineNumber << ": ";
    }
  ss << this->m_records[i].message << std::endl;
  return ss.str();
}

/**\brief Convert the given log entry range to a string.
  *
  */
std::string Logger::toString(std::size_t i, std::size_t j) const
{
  std::stringstream ss;
  for (; i < j; i++)
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

//----------------------------------------------------------------------------
std::string Logger::convertToString() const
{
  return this->toString(0, this->m_records.size());
}

/**\brief Request all future records be flushed to \a output as they are logged.
  *
  * If \a ownFile is true, then the Logger takes ownership of \a output
  * and will delete it when the Logger is destructed.
  * If \a output is NULL, then this stops future log records from
  * being appended to any file.
  */
void Logger::setFlushToStream(std::ostream* output, bool ownFile)
{
  if (this->m_ownFile)
    delete this->m_file;
  this->m_file = output;
  this->m_ownFile = output ? ownFile : false;
}

  } // namespace io
} // namespace smtk

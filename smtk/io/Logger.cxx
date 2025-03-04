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

#include <fstream>
#include <iostream>

namespace smtk
{
namespace io
{
Logger Logger::m_instance;

Logger& Logger::instance()
{
  return Logger::m_instance;
}

Logger::~Logger()
{
  this->setFlushToStream(nullptr, false, false);
  if (m_callback)
  {
    m_callback();
  }
}

Logger& Logger::operator=(const Logger& logger)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_records = logger.m_records;
  m_hasErrors = logger.m_hasErrors;
  return *this;
}

void Logger::addRecord(
  Severity s,
  const std::string& m,
  const std::string& fname,
  unsigned int line)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if ((s == Logger::Error) || (s == Logger::Fatal))
  {
    m_hasErrors = true;
  }
  m_records.emplace_back(s, m, fname, line);
  std::size_t nr = this->numberOfRecords();
  this->flushRecordsToStream(nr - 1, nr);
}

void Logger::append(const Logger& l)
{
  // do not append a logger to itself
  if (&l == this)
  {
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  m_records.insert(m_records.end(), l.m_records.begin(), l.m_records.end());
  if (l.m_hasErrors)
  {
    m_hasErrors = true;
  }
  std::size_t nr = this->numberOfRecords();
  this->flushRecordsToStream(nr - l.numberOfRecords(), nr);
}

void Logger::reset()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_hasErrors = false;
  m_records.clear();
}

std::vector<Logger::Record> Logger::records() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_records;
}

Logger::Record Logger::record(std::size_t i) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_records[i];
}

std::string Logger::severityAsString(Severity s)
{
  switch (s)
  {
    case Debug:
      return "DEBUG";
    case Info:
      return "INFO";
    case Warning:
      return "WARNING";
    case Error:
      return "ERROR";
    case Fatal:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
  return "UNKNOWN";
}

/**\brief Convert the given log entry to a string.
  *
  */
std::string Logger::toString(const Logger::Record& record, bool includeSourceLoc)
{
  std::stringstream ss;
  ss << severityAsString(record.severity) << ": ";
  if (includeSourceLoc && !record.fileName.empty())
  {
    ss << "In " << record.fileName << ", line " << record.lineNumber << ": ";
  }
  ss << record.message << std::endl;
  return ss.str();
}

/**\brief Convert the given log entry to a string.
  *
  */
std::string Logger::toString(std::size_t i, bool includeSourceLoc) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return Logger::toString(m_records[i], includeSourceLoc);
}

/**\brief Convert the given log entry range to a string.
  *
  */
std::string Logger::toString(std::size_t i, std::size_t j, bool includeSourceLoc) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return this->toStringInternal(i, j, includeSourceLoc);
}

std::string Logger::toStringInternal(std::size_t i, std::size_t j, bool includeSourceLoc) const
{
  std::stringstream ss;
  for (; i < j; i++)
  {
    ss << severityAsString(m_records[i].severity) << ": ";
    if (includeSourceLoc && !m_records[i].fileName.empty())
    {
      ss << "In " << m_records[i].fileName << ", line " << m_records[i].lineNumber << ": ";
    }
    ss << m_records[i].message << std::endl;
  }
  return ss.str();
}

std::string Logger::toHTML(std::size_t i, std::size_t j, bool includeSourceLoc) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::stringstream ss;
  ss << "<table>";
  for (; i < j; i++)
  {
    ss << "<tr class=\"" << severityAsString(m_records[i].severity) << "\">";
    if (includeSourceLoc)
    {
      ss << "<td>";
      if (!m_records[i].fileName.empty())
      {
        ss << m_records[i].fileName << ", line " << m_records[i].lineNumber;
      }
      else
      {
        ss << "&nbsp;";
      }
      ss << "</td>";
    }
    ss << "<td>" << m_records[i].message << "</td></tr>\n";
  }
  ss << "</table>";
  return ss.str();
}

std::string Logger::convertToString(bool includeSourceLoc) const
{
  return this->toString(0, m_records.size(), includeSourceLoc);
}

std::string Logger::convertToHTML(bool includeSourceLog) const
{
  return this->toHTML(0, m_records.size(), includeSourceLog);
}

/**\brief Request all records be flushed to \a output as they are logged.
  *
  * If \a ownFile is true, then the Logger takes ownership of \a output
  * and will delete it when the Logger is destructed.
  * If \a output is nullptr, then this stops future log records from
  * being appended to any file.
  * If \a includePast is true, then all pre-existing records are
  * written to the stream before this method returns (and future
  * records are written as they are added/appended).
  *
  * Note that only a single stream will be written at a time;
  * calling this or other "setFlushTo" methods multiple times
  * will only change where new records are written.
  */
void Logger::setFlushToStream(std::ostream* output, bool ownFile, bool includePast)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_ownStream)
    delete m_stream;
  m_stream = output;
  m_ownStream = output ? ownFile : false;
  if (includePast)
    this->flushRecordsToStream(0, this->numberOfRecords());
}

/**\brief Request all records be flushed to a file with the given \a filename.
  *
  * If \a includePast is true, then all pre-existing records will
  * be immediately written to the file (and future records as they
  * are added/appended).
  *
  * Returns true if the file was successfully created.
  *
  * Note that only a single stream will be written at a time;
  * calling this or other "setFlushTo" methods multiple times
  * will only change where new records are written.
  */
bool Logger::setFlushToFile(std::string filename, bool includePast)
{
  std::ofstream* file = new std::ofstream(filename.c_str(), std::ios::app);
  if (file->good())
  {
    this->setFlushToStream(file, true, includePast);
  }
  else
  {
    delete file;
    return false;
  }
  return true;
}

/**\brief Request all records be flushed to the process's standard output.
  *
  * If \a includePast is true, then all pre-existing records will
  * be immediately written to the file (and future records as they
  * are added/appended).
  *
  * This method is a convenience for Python users.
  *
  * Note that only a single stream will be written at a time;
  * calling this or other "setFlushTo" methods multiple times
  * will only change where new records are written.
  */
void Logger::setFlushToStdout(bool includePast)
{
  this->setFlushToStream(&std::cout, false, includePast);
}

/**\brief Request all records be flushed to the process's standard error output.
  *
  * If \a includePast is true, then all pre-existing records will
  * be immediately written to the file (and future records as they
  * are added/appended).
  *
  * This method is a convenience for Python users.
  *
  * Note that only a single stream will be written at a time;
  * calling this or other "setFlushTo" methods multiple times
  * will only change where new records are written.
  */
void Logger::setFlushToStderr(bool includePast)
{
  this->setFlushToStream(&std::cerr, false, includePast);
}

/// Set a function to be called upon the destruction of the logger.
void Logger::setCallback(std::function<void()> fn)
{
  m_callback = fn;
}

/// This is a helper routine to write records to the stream (if one has been set).
void Logger::flushRecordsToStream(std::size_t beginRec, std::size_t endRec)
{
  if (m_stream && beginRec < endRec && beginRec < numberOfRecords() && endRec <= numberOfRecords())
  {
    (*m_stream) << this->toStringInternal(beginRec, endRec);
    m_stream->flush();
  }
}

} // namespace io
} // namespace smtk

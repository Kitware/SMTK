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

namespace smtk {
  namespace io {

Logger::~Logger()
{
  this->setFlushToStream(NULL, false, false);
}

void Logger::addRecord(Severity s, const std::string &m,
                       const std::string &fname,
                       unsigned int line)
{
  if ((s == Logger::ERROR) || (s == Logger::FATAL))
    {
    this->m_hasErrors = true;
    }
  this->m_records.push_back(Record(s, m, fname, line));
  std::size_t nr = this->numberOfRecords();
  this->flushRecordsToStream(nr - 1, nr);
}

void Logger::append(const Logger &l)
{
  this->m_records.insert(this->m_records.end(), l.m_records.begin(),
                         l.m_records.end());
  if (l.m_hasErrors)
    {
    this->m_hasErrors = true;
    }
  std::size_t nr = this->numberOfRecords();
  this->flushRecordsToStream(nr - l.numberOfRecords(), nr);
}

void Logger::reset()
{
  this->m_hasErrors = false;
  this->m_records.empty();
}

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
std::string Logger::toString(std::size_t i, bool includeSourceLoc) const
{
  std::stringstream ss;
  ss << severityAsString(this->m_records[i].severity) << ": ";
  if (includeSourceLoc && this->m_records[i].fileName != "")
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
std::string Logger::toString(std::size_t i, std::size_t j, bool includeSourceLoc) const
{
  std::stringstream ss;
  for (; i < j; i++)
    {
    ss << severityAsString(this->m_records[i].severity) << ": ";
    if (includeSourceLoc && this->m_records[i].fileName != "")
      {
      ss << "In " << this->m_records[i].fileName << ", line "
         << this->m_records[i].lineNumber << ": ";
      }
    ss << this->m_records[i].message << std::endl;
    }
  return ss.str();
}

std::string Logger::toHTML(std::size_t i, std::size_t j, bool includeSourceLoc) const
{
  std::stringstream ss;
  ss << "<table>";
  for (; i < j; i++)
    {
    ss << "<tr class=\"" << severityAsString(this->m_records[i].severity) << "\">";
    if (includeSourceLoc)
      {
      ss << "<td>";
      if (this->m_records[i].fileName != "")
        {
        ss << this->m_records[i].fileName << ", line " << this->m_records[i].lineNumber;
        }
      else
        {
        ss << "&nbsp;";
        }
      ss << "</td>";
      }
    ss << "<td>" << this->m_records[i].message << "</td></tr>\n";
    }
  ss << "</table>";
  return ss.str();
}

std::string Logger::convertToString(bool includeSourceLoc) const
{
  return this->toString(0, this->m_records.size(), includeSourceLoc);
}

std::string Logger::convertToHTML(bool includeSourceLog) const
{
  return this->toHTML(0, this->m_records.size(), includeSourceLog);
}

/**\brief Request all records be flushed to \a output as they are logged.
  *
  * If \a ownFile is true, then the Logger takes ownership of \a output
  * and will delete it when the Logger is destructed.
  * If \a output is NULL, then this stops future log records from
  * being appended to any file.
  * If \a includePast is true, then all pre-existing records are
  * written to the stream before this method returns (and future
  * records are written as they are added/appended).
  *
  * Note that only a single stream will be written at a time;
  * calling this or other "setFlushTo" methods multiple times
  * will only change where new records are written.
  */
void Logger::setFlushToStream(
  std::ostream* output, bool ownFile, bool includePast)
{
  if (this->m_ownStream)
    delete this->m_stream;
  this->m_stream = output;
  this->m_ownStream = output ? ownFile : false;
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
bool Logger::setFlushToFile( std::string filename, bool includePast)
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

/// This is a helper routine to write records to the stream (if one has been set).
void Logger::flushRecordsToStream(std::size_t beginRec, std::size_t endRec)
{
  if (
    this->m_stream &&
    beginRec < endRec &&
    beginRec < numberOfRecords() &&
    endRec <= numberOfRecords())
    {
    (*this->m_stream) << this->toString(beginRec, endRec);
    this->m_stream->flush();
    }
}

  } // namespace io
} // namespace smtk

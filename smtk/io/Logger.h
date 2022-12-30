//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_io_Logger_h
#define smtk_io_Logger_h
/*! \file */

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"
#include <functional>
#include <iosfwd>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#ifdef DEBUG
#undef DEBUG // Some libraries publicly export this define when built in debug mode
// That messes up the DEBUG enumeration in Severity.
#endif
#ifdef ERROR // Same
#undef ERROR
#endif
#ifdef WARNING // Same
#undef WARNING
#endif

/**\brief Write the expression \a x to \a logger as an error message.
  *
  * Note that \a x may use the "<<" operator.
  */
#define smtkErrorMacro(logger, x)                                                                  \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x; /* NOLINT(bugprone-macro-parentheses) */                                              \
    (logger).addRecord(smtk::io::Logger::Error, s1.str(), __FILE__, __LINE__);                     \
  } while (0)

/**\brief Write the expression \a x to \a logger as a warning message.
  *
  * Note that \a x may use the "<<" operator.
  */
#define smtkWarningMacro(logger, x)                                                                \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x; /* NOLINT(bugprone-macro-parentheses) */                                              \
    (logger).addRecord(smtk::io::Logger::Warning, s1.str(), __FILE__, __LINE__);                   \
  } while (0)

/**\brief Write the expression \a x to \a logger as a debug message.
  *
  * Note that \a x may use the "<<" operator.
  */
#define smtkDebugMacro(logger, x)                                                                  \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x; /* NOLINT(bugprone-macro-parentheses) */                                              \
    (logger).addRecord(smtk::io::Logger::Debug, s1.str(), __FILE__, __LINE__);                     \
  } while (0)

/**\brief Write the expression \a x to \a logger as an informational message.
  *
  * Note that \a x may use the "<<" operator.
  *
  * Unlike other logging macros, this does not include  a
  * filename and line number in the record.
  */
#define smtkInfoMacro(logger, x)                                                                   \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x; /* NOLINT(bugprone-macro-parentheses) */                                              \
    (logger).addRecord(smtk::io::Logger::Info, s1.str());                                          \
  } while (0)

namespace smtk
{
namespace io
{

/**\brief Log messages for later presentation to a user or a file.
 *
 * Logger has a singleton interface to a global logger, but is also
 * constructible as a non-singleton object.
 */
class SMTKCORE_EXPORT Logger
{
public:
  static Logger& instance();

  enum Severity
  {
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
  };

  struct Record
  {
    Severity severity{ Info };
    std::string message;
    std::string fileName;
    unsigned int lineNumber{ 0 };
    Record(Severity s, const std::string& m, const std::string& f = "", unsigned int l = 0)
      : severity(s)
      , message(m)
      , fileName(f)
      , lineNumber(l)
    {
    }
    Record() = default;
  };

  Logger() = default;

  Logger(const Logger& logger)
    : m_hasErrors(logger.m_hasErrors)
    , m_records(logger.m_records)
  {
  }

  virtual ~Logger();

  Logger& operator=(const Logger& logger);
  std::size_t numberOfRecords() const { return m_records.size(); }

  bool hasErrors() const { return m_hasErrors; }
  void clearErrors() { m_hasErrors = false; }

  void
  addRecord(Severity s, const std::string& m, const std::string& fname = "", unsigned int line = 0);

  ///\brief Return a copy of all the records contained within the Logger
  ///
  /// Note - the reason a copy of the records is returned instead of a reference is to make
  /// the call threadsafe
  std::vector<Record> records() const;
  ///\brief Return a copy of the ith record in the logger
  Record record(std::size_t i) const;

  static std::string toString(const Record& record, bool includeSourceLoc = false);
  std::string toString(std::size_t i, bool includeSourceLoc = false) const;
  std::string toString(std::size_t i, std::size_t j, bool includeSourceLoc = false) const;

  std::string toHTML(std::size_t i, std::size_t j, bool includeSourceLoc) const;

  // Convert all the messages into a single string
  std::string convertToString(bool includeSourceLoc = false) const;
  std::string convertToHTML(bool includeSourceLoc = false) const;
  void reset();

  void append(const Logger& l);

  static std::string severityAsString(Severity s);

  void setFlushToStream(std::ostream* output, bool ownFile, bool includePast);
  bool setFlushToFile(std::string filename, bool includePast);
  void setFlushToStdout(bool includePast);
  void setFlushToStderr(bool includePast);

  void setCallback(std::function<void()> fn);

protected:
  void flushRecordsToStream(std::size_t beginRec, std::size_t endRec);
  std::string toStringInternal(std::size_t i, std::size_t j, bool includeSourceLoc = false) const;

  bool m_hasErrors{ false };
  std::vector<Record> m_records;
  std::ostream* m_stream{ nullptr };
  bool m_ownStream{ false };
  std::function<void()> m_callback;

private:
  static Logger m_instance;
  mutable std::mutex m_mutex;
};

template<typename J>
void to_json(J& json, const Logger::Record& rec)
{
  json = { { "severity", rec.severity },
           { "message", rec.message },
           { "file", rec.fileName },
           { "line", rec.lineNumber } };
}
} // namespace io
} // namespace smtk

#endif /* smtk_io_Logger_h */

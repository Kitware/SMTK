//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_LogParser_h
#define smtk_job_LogParser_h

#include "smtk/SharedFromThis.h"
#include "smtk/common/Observers.h"

#include <functional>
#include <memory>

namespace smtk
{
namespace job
{

/// A functor called when information records extracted from a log become available.
///
/// The arguments passed to the functor are
/// + the log file
/// + the type of information records
/// + the first modified/created record
/// + just past the last modified/created record.
using LogObserver =
  std::function<void(smtk::string::Token, smtk::string::Token, std::size_t, std::size_t)>;

/// A class for registering observers of log information.
using LogObservers = smtk::common::Observers<LogObserver>;

/// A specification for a function that can incrementally parse a log file.
///
/// It is better for simulations to produce Catalyst data than log files,
/// but if a simulation cannot be instrumented with Catalyst then a log
/// file parser may be easier to implement.
///
/// Subclasses of this class should be registered with the
/// job::Resource::instance() so they can be instantiated as specified
/// by operations producing input decks.
class SMTKCORE_EXPORT LogParser : smtkEnableSharedPtr(LogParser)
{
public:
  smtkTypeMacroBase(smtk::job::LogParser);
  smtkSharedFromThisMacro(smtk::job::LogParser);
  smtkCreateMacro(smtk::job::LogParser);

  LogParser() = default;
  virtual ~LogParser() = default;

  ///@{
  /// Set/get the name of the log file being parsed.
  smtk::string::Token logName() const { return m_logName; }
  bool setLogName(smtk::string::Token logName)
  {
    if (m_logName == logName || !logName.valid())
    {
      return false;
    }
    m_logName = logName;
    return true;
  }
  ///@}

  /// Subclasses should override this method to parse \a data as it is ingested.
  ///
  /// If parsing \a data produces new summary information, subclasses should
  /// call summaryUpdated() as needed from within processBytes.
  /// This will trigger observers who may then request the summary information.
  virtual void processBytes(const char* data, std::size_t length)
  {
    (void)data;
    (void)length;
  }

  /// Subclasses should override this method to provide a list of information keys
  /// which may be queried as parsing proceeds.
  virtual std::unordered_set<smtk::string::Token> informationKeys() const
  {
    return std::unordered_set<smtk::string::Token>();
  }

  /// Subclasses should override this method to provide the number of records
  /// available for the given \a informationKey.
  virtual std::size_t summaryRecordCount(smtk::string::Token informationKey)
  {
    (void)informationKey;
    return 0;
  }

  /// Subclasses should override this method to provide metadata about
  /// summary information.
  ///
  /// The returned JSON should describe how the data should be presented
  /// to users. For example, time-series data such as the simulation's current
  /// time, time-step-count, and convergence metrics should describe themselves
  /// as unbounded series data while things like iteration numbers that are cyclical
  /// should describe themselves as state updates.
  virtual nlohmann::json summaryMetadata(smtk::string::Token informationKey) const
  {
    (void)informationKey;
    return nlohmann::json();
  }

  /// Subclasses should call this method from within their override of summaryUpdated.
  ///
  /// Calling this method indicates that the summary information corresponding to
  /// \a informationKey has been added or updated in the range of records in
  /// the half-open interval [\a summaryBegin, \a summaryEnd [.
  /// It also indicates that the log has been consumed up to the given \a byteOffset.
  ///
  /// The \a byteOffset is important because if the client is restarted while a job
  /// is still running, the queue will start passing log data to the parser starting
  /// at the byte offset.
  ///
  /// Generally, calling this method will result in clients calling \a fetchSummary()
  /// to obtain the new summary information.
  void summaryUpdated(
    smtk::string::Token informationKey,
    std::size_t summaryBegin,
    std::size_t summaryEnd,
    std::size_t byteOffset)
  {
    if (byteOffset > m_byteOffset)
    {
      m_byteOffset = byteOffset;
    }
    m_observers(m_logName, informationKey, summaryBegin, summaryEnd);
  }

  /// Subclasses should override this method to return summary records (in the
  /// half-open interval [\a begin , \a end [ ) for the given \a informationKey.
  virtual nlohmann::json
  fetchSummary(smtk::string::Token informationKey, std::size_t begin, std::size_t end)
  {
    (void)informationKey;
    (void)begin;
    (void)end;
    return nlohmann::json();
  }

  ///@{
  /// Return the observers for this parser so clients can insert themselves.
  const LogObservers& observers() const { return m_observers; }
  LogObservers& observers() { return m_observers; }
  ///@}

protected:
  std::size_t m_byteOffset{ 0 };
  smtk::string::Token m_logName;
  LogObservers m_observers;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_LogParser_h

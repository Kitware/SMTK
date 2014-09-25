/*=========================================================================

Copyright (c) 1998-2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
#ifndef __smtk_util_Logger_h
#define __smtk_util_Logger_h
/*! \file */

#include "smtk/SMTKCoreExports.h"
#include "smtk/util/SystemConfig.h"
#include <string>
#include <sstream>
#include <vector>

/// Write the expression \a x (which may use the "<<" operator) to \a logger as an error message.
#define smtkErrorMacro(logger, x) do {                  \
  std::stringstream s1;                                 \
  s1 << x;                                              \
  logger.addRecord(smtk::util::Logger::ERROR,           \
                   s1.str(),  __FILE__,  __LINE__);     \
  } while (0)

/// Write the expression \a x (which may use the "<<" operator) to \a logger as a warning message.
#define smtkWarningMacro(logger, x) do {                \
  std::stringstream s1;                                 \
  s1 << x;                                              \
  logger.addRecord(smtk::util::Logger::WARNING,         \
                   s1.str(),  __FILE__,  __LINE__);     \
  } while (0)

/// Write the expression \a x (which may use the "<<" operator) to \a logger as a debug message.
#define smtkDebugMacro(logger, x) do {                  \
  std::stringstream s1;                                 \
  s1 << x;                                              \
  logger.addRecord(smtk::util::Logger::DEBUG,           \
                   s1.str(),  __FILE__,  __LINE__);     \
  } while (0)
///@}

namespace smtk
{
  namespace util
  {

    /**\brief Log messages for later presentation to a user or a file.
      *
      */
    class SMTKCORE_EXPORT Logger
    {
    public:
      enum Severity
      {DEBUG, INFO, WARNING, ERROR, FATAL};

      struct Record
      {
        Severity severity;
        std::string message;
        std::string fileName;
        unsigned int lineNumber;
        Record(Severity s, const std::string &m,
               const std::string &f="", unsigned int l=0):
          severity(s), message(m), fileName(f), lineNumber(l) {}
        Record():
          severity(INFO), lineNumber(0) {}
      };

      Logger(): m_hasErrors(false) {}
      std::size_t numberOfRecords() const
      {return this->m_records.size();}

      bool hasErrors() const
      {return this->m_hasErrors;}

      void addRecord(Severity s, const std::string &m,
                     const std::string &fname="",
                     unsigned int line=0);

      const Record &record(int i) const
      {return this->m_records[i];}

      // Convert all the messages into a single string
      std::string convertToString() const;
      void reset();

      void append(const Logger &l);

      static std::string severityAsString(Severity s);

    protected:
      std::vector<Record> m_records;
      bool m_hasErrors;
    private:

    };
  };
};

#endif /* __smtk_util_Logger_h */

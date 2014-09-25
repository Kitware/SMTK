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

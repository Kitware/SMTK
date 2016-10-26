//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/DateTime.h"

#ifndef SHIBOKEN_SKIP
SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/time_parsing.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
SMTK_THIRDPARTY_POST_INCLUDE
#endif

#include <exception>

namespace smtk {
  namespace attribute {

//----------------------------------------------------------------------------
/// Default constructor creates invalid ptime
DateTime::DateTime()
  : m_data()
{
}

//----------------------------------------------------------------------------
bool DateTime::isValid() const
{
  return !this->m_data.is_special();
}

//----------------------------------------------------------------------------
bool DateTime::parseIsoString(const std::string& ts)
{
  try
    {
    std::string tsCopy(ts);
//tsCopy.erase(std::remove(tsCopy.begin(), tsCopy.end(), '-'), tsCopy.end());
    this->m_data = boost::posix_time::from_iso_string(tsCopy);
    }
  catch (std::exception& e)
    {
#ifndef NDEBUG
    std::cerr << "exception: " << e.what() << std::endl;
#endif
    this->m_data = boost::posix_time::not_a_date_time;
    return false;
    }

  return this->isValid();
}

//----------------------------------------------------------------------------
bool DateTime::parseString(const std::string& ts)
{
  this->m_data = boost::posix_time::time_from_string(ts);
  return this->isValid();

}

  }  // namespace attribute
}  // namespace smtk

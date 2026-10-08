//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_db_SingleValueContainer_h
#define smtk_job_db_SingleValueContainer_h

#include "smtk/job/db/Binding.h"

namespace smtk
{
namespace job
{
namespace db
{

template<typename ValueType>
class sqlSingleValueContainer
{
public:
  using value_type = ValueType;

  sqlSingleValueContainer(ValueType& value)
    : m_value(&value)
  {
  }

  std::size_t end() const { return 0; }

  std::size_t size() const { return m_set ? 1 : 0; }
  template<typename Iter>
  Iter insert(Iter location, const ValueType& value)
  {
    if (!m_set)
    {
      *m_value = value;
      m_set = true;
    }
    return location;
  }

protected:
  bool m_set{ false };
  ValueType* m_value{ nullptr };
};

} // namespace db
} // namespace job
} // namespace smtk

#endif // smtk_job_db_SingleValueContainer_h

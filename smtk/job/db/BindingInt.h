//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_db_BindingInt_h
#define smtk_job_db_BindingInt_h

#include "smtk/job/db/Query.h"
#include "smtk/job/db/SingleValueContainer.h"

#include "sqlite3.h"

namespace smtk
{
namespace job
{
namespace db
{

template<typename IntegerType, typename Container = sqlSingleValueContainer<IntegerType>>
class sqlBindingInt : public sqlBinding
{
public:
  using container_type = Container;
  sqlBindingInt(std::size_t index, const Container& values, std::size_t maxSize = 0)
    : sqlBinding(index, maxSize)
    , m_values(const_cast<Container*>(&values))
  {
  }

  bool captureValue(sqlQuery& query) override
  {
    if (this->maximumSize() && m_values->size() >= this->maximumSize())
    {
      return false;
    }
    m_values->insert(
      m_values->end(),
      static_cast<IntegerType>(sqlite3_column_int(query.cursor(), static_cast<int>(m_index))));
    return true;
  }

protected:
  Container* m_values{ nullptr };
};

} // namespace db
} // namespace job
} // namespace smtk

#endif // smtk_job_db_BindingInt_h

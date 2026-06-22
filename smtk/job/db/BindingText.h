//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_db_BindingText_h
#define smtk_job_db_BindingText_h

#include "smtk/job/db/SingleValueContainer.h"

#include "sqlite3.h"

namespace smtk
{
namespace job
{
namespace db
{

template<typename TextType, typename Container = sqlSingleValueContainer<TextType>>
class sqlBindingText : public sqlBinding
{
public:
  using container_type = Container;
  sqlBindingText(std::size_t index, const Container& values, std::size_t maxSize = 0)
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
      TextType(reinterpret_cast<const char*>(
        sqlite3_column_text(query.cursor(), static_cast<int>(m_index)))));
    return true;
  }

protected:
  Container* m_values{ nullptr };
};

} // namespace db
} // namespace job
} // namespace smtk

#endif // smtk_job_db_BindingText_h

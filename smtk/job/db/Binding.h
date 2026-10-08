//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_db_Binding_h
#define smtk_job_db_Binding_h

#include "smtk/CoreExports.h"

#include <cstddef>

namespace smtk
{
namespace job
{
namespace db
{

class sqlQuery;

class sqlBinding
{
public:
  sqlBinding(std::size_t index, std::size_t maximumSize = 0)
    : m_index(index)
    , m_maximumSize(maximumSize)
  {
  }

  virtual bool captureValue(sqlQuery&) = 0;

  std::size_t index() const { return m_index; }
  std::size_t maximumSize() const { return m_maximumSize; }

protected:
  std::size_t m_index{ std::size_t(~0) };
  std::size_t m_maximumSize{ 0 };
};

template<typename IntegerType, typename Container>
class sqlBindingInt;
template<typename IntegerType, typename Container>
class sqlBindingText;

} // namespace db
} // namespace job
} // namespace smtk

#endif // smtk_job_db_Binding_h

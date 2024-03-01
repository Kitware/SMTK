//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/IndirectAssignedIds.h"

#include "smtk/markup/IdSpace.h"

#include "smtk/string/Token.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace markup
{

AssignedIds::AssignedIdCtor IndirectAssignedIds::cloneFunctor() const
{
  AssignedIdCtor ctor =
    [&](const std::shared_ptr<IdSpace>& space, IdNature nature, IdType begin, IdType end) {
      auto data = std::make_shared<IndirectAssignedIds>(space, nature, begin, end, nullptr);
      std::shared_ptr<AssignedIds> result = data;
      return result;
    };
  return ctor;
}

void IndirectAssignedIds::setIdArray(vtkSmartPointer<vtkIdTypeArray> idArray)
{
  m_idToIndex.clear();
  m_idArray = idArray;
  if (m_idArray)
  {
    auto rr = this->range();
    auto cc = m_idArray->GetNumberOfComponents();
    if (cc != 1)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "ID array must have a single component, not " << cc << ".");
    }
    else
    {
      vtkIdType nn = m_idArray->GetNumberOfTuples();
      for (vtkIdType ii = 0; ii < nn; ++ii)
      {
        auto vv = static_cast<IdType>(m_idArray->GetValue(ii));
        if (vv < rr[0] || vv >= rr[1])
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "ID " << vv << " (index " << ii
                  << ") out of "
                     "range ["
                  << rr[0] << ", " << rr[1] << "[. Ignoring.");
          continue;
        }
        m_idToIndex.insert(std::make_pair(vv, ii));
      }
    }
  }
}

IdType IndirectAssignedIds::size() const
{
  IdType result = 0;
  for (auto it = m_idToIndex.begin(); it != m_idToIndex.end();
       it = m_idToIndex.upper_bound(it->first))
  {
    ++result;
  }
  return result;
}

IdType IndirectAssignedIds::maxId() const
{
  IdType result = this->size();
  --result;
  return result;
}

bool IndirectAssignedIds::empty() const
{
  return m_idToIndex.empty();
}

IndirectAssignedIds::Iterator<IndirectAssignedIds::Forward> IndirectAssignedIds::begin() const
{
  Iterator<Forward> result(
    m_idToIndex.empty() ? Iterator<Forward>::Invalid : m_idToIndex.begin()->first,
    std::static_pointer_cast<const IndirectAssignedIds>(shared_from_this()));
  return result;
}

IndirectAssignedIds::Iterator<IndirectAssignedIds::Forward> IndirectAssignedIds::end() const
{
  Iterator<Forward> result(
    Iterator<Forward>::Invalid,
    std::static_pointer_cast<const IndirectAssignedIds>(shared_from_this()));
  return result;
}

IndirectAssignedIds::Iterator<IndirectAssignedIds::Reverse> IndirectAssignedIds::rbegin() const
{
  Iterator<Reverse> result(
    m_idToIndex.empty() ? Iterator<Reverse>::Invalid : m_idToIndex.rbegin()->first,
    std::static_pointer_cast<const IndirectAssignedIds>(shared_from_this()));
  return result;
}

IndirectAssignedIds::Iterator<IndirectAssignedIds::Reverse> IndirectAssignedIds::rend() const
{
  Iterator<Reverse> result(
    Iterator<Reverse>::Invalid,
    std::static_pointer_cast<const IndirectAssignedIds>(shared_from_this()));
  return result;
}

IdType IndirectAssignedIds::contains(IdType begin, IdType end) const
{
  IdType result = 0;
  for (auto it = m_idToIndex.lower_bound(begin); it != m_idToIndex.end() && it->first < end;
       it = m_idToIndex.upper_bound(it->first))
  {
    ++result;
  }
  return result;
}

vtkIdType IndirectAssignedIds::firstIndexFor(IdType value) const
{
  auto it = m_idToIndex.lower_bound(value);
  if (it == m_idToIndex.end() || it->first != value)
  {
    return Iterator<Forward>::Invalid;
  }
  return it->second;
}

} // namespace markup
} // namespace smtk

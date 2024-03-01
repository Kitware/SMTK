//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/AssignedIds.h"

#include "smtk/markup/IdSpace.h"

#include "smtk/string/Token.h"

namespace smtk
{
namespace markup
{

AssignedIds::~AssignedIds()
{
  // Always remove ourselves from our containing IdSpace.
  if (auto idspace = m_space.lock())
  {
    idspace->removeEntry(*this);
  }
}

AssignedIds::AssignedIdCtor AssignedIds::cloneFunctor() const
{
  AssignedIdCtor ctor =
    [&](const std::shared_ptr<IdSpace>& space, IdNature nature, IdType begin, IdType end) {
      return std::make_shared<AssignedIds>(space, nature, begin, end, nullptr);
    };
  return ctor;
}

AssignedIds::IdRange AssignedIds::range() const
{
  return m_range;
}

#if 0
/// Returns a functor to iterate all assigned IDs. Call the functor until it returns false.
Iterable AssignedIds::iterable() const;
  /// Returns functor to query the number of allotted ids in the half-open interval [begin, end[.
ContainsFunctor AssignedIds::contains() const;
  /// Call \a visitor on each alloted ID.
smtk::common::Visit visit(Visitor visitor) const;
#endif

std::shared_ptr<smtk::markup::IdSpace> AssignedIds::space() const
{
  return m_space.lock();
}

bool AssignedIds::setNature(const IdNature& nature)
{
  if (m_nature == nature)
  {
    return false;
  }
  m_nature = nature;
  return true;
}

const IdNature& AssignedIds::nature() const
{
  return m_nature;
}

IdNature& AssignedIds::nature()
{
  return m_nature;
}

} // namespace markup
} // namespace smtk

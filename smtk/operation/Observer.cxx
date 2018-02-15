//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Observer.h"

#include <iostream>

namespace smtk
{
namespace operation
{

int Observers::operator()(std::shared_ptr<Operation> op, EventType event, Operation::Result opres)
{
  int result = 0;
  if (!op || (!opres && event == EventType::DID_OPERATE))
  {
    std::cerr << "Error: operation events must have an operator (and sometimes a result).\n";
    return result;
  }

  // This careful loop allows an observer to erase itself.
  std::map<Key, Observer>::iterator entry = m_observers.begin();
  std::map<Key, Observer>::iterator next;
  for (next = entry; entry != m_observers.end(); entry = next)
  {
    ++next;
    result |= entry->second(op, event, opres);
  }
  return result;
}

Observers::Key Observers::insert(Observer fn)
{
  Key handle = m_observers.empty() ? 0 : m_observers.rbegin()->first + 1;
  return m_observers.insert(std::make_pair(handle, fn)).second ? handle : -1;
}

std::size_t Observers::erase(Observers::Key handle)
{
  return m_observers.erase(handle);
}

Observer Observers::find(Key handle) const
{
  auto entry = m_observers.find(handle);
  return entry == m_observers.end() ? nullptr : entry->second;
}

} // operation namespace
} // smtk namespace

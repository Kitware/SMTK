//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/Observer.h"

#include <iostream>

namespace smtk
{
namespace resource
{

void Observers::operator()(std::shared_ptr<Resource> resource, EventType event)
{
  if (!resource)
  {
    std::cerr << "Error: resource events must have a resource.\n";
    return;
  }

  // This careful loop allows an observer to erase itself.
  std::map<Key, Observer>::iterator entry = m_observers.begin();
  std::map<Key, Observer>::iterator next;
  for (next = entry; entry != m_observers.end(); entry = next)
  {
    ++next;
    entry->second(resource, event);
  }
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

} // resource namespace
} // smtk namespace

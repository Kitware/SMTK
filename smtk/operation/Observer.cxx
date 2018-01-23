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

int Observers::operator()(std::shared_ptr<NewOp> op, EventType event, NewOp::Result opres)
{
  int result = 0;
  if (!op || (!opres && event == EventType::DID_OPERATE))
  {
    std::cerr << "Error: operation events must have an operator (and sometimes a result).\n";
    return result;
  }

  for (auto entry : m_observers)
  {
    result |= entry.second(op, event, opres);
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

} // operation namespace
} // smtk namespace

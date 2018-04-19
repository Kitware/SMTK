//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/workflow/OperationFilterSort.h"

#include <algorithm>

using namespace smtk::workflow;

OperationFilterSort::OperationFilterSort()
{
}

OperationFilterSort::~OperationFilterSort()
{
}

void OperationFilterSort::apply(const WorkingSet& workingSet, Output& operationsToDisplay)
{
  operationsToDisplay.clear();
  for (auto entry : workingSet)
  {
    if (m_filterList.find(entry) == m_filterList.end())
    {
      continue;
    }

    operationsToDisplay.push_back(entry);
  }
  std::sort(operationsToDisplay.begin(), operationsToDisplay.end(), [this](const Index& a,
                                                                      const Index& b) -> bool {
    const auto& da = m_filterList[a];
    const auto& db = m_filterList[b];
    return da.precedence < db.precedence || (da.precedence == db.precedence && (da.name < db.name));
  });
}

OperationFilterSort::ObserverKey OperationFilterSort::observe(Observer fn, bool invokeImmediately)
{
  ObserverKey key = m_observers.empty() ? 0 : (m_observers.rbegin()->first + 1);
  m_observers.insert(std::make_pair(key, fn));
  if (invokeImmediately)
  {
    fn();
  }
  return key;
}

bool OperationFilterSort::unobserve(const ObserverKey& id)
{
  return m_observers.erase(id) > 0;
}

void OperationFilterSort::triggerObservers() const
{
  for (auto& entry : m_observers)
  {
    entry.second();
  }
}

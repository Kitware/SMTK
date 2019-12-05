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

OperationFilterSort::OperationFilterSort() = default;

OperationFilterSort::~OperationFilterSort() = default;

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

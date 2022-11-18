//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/json/jsonNameManager.h"

namespace smtk
{
namespace view
{

void to_json(nlohmann::json& jj, const smtk::view::NameManager::Ptr& nameManager)
{
  if (!nameManager)
  {
    return;
  }
  jj["counter"] = nameManager->counter();
}

void from_json(const nlohmann::json& jj, smtk::view::NameManager::Ptr& nameManager)
{
  if (!nameManager)
  {
    nameManager = smtk::view::NameManager::create();
  }
  auto it = jj.find("counter");
  if (it != jj.end())
  {
    int currentCounter = nameManager->counter();
    int jsonCounter = it->get<int>();
    // Never decrement the counter in memory; only increment it.
    if (jsonCounter > currentCounter)
    {
      nameManager->resetCounter(jsonCounter);
    }
  }
}

} // namespace view
} // namespace smtk

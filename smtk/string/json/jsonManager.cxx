//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/string/json/jsonManager.h"

namespace smtk
{
namespace string
{

void to_json(json& j, const std::shared_ptr<Manager>& m)
{
  json::object_t members;
  json::object_t sets;

  m->visitMembers([&members, &m](Hash h) {
    // We store the "inverse" of the string-manager's map because
    // JSON keys must be strings and we would rather avoid the overhead
    // and potential ambiguities of conversion where possible, although
    // the sets below do some conversion.
    members[m->value(h)] = h;
    return smtk::common::Visit::Continue;
  });
  j["members"] = members;

  m->visitSets([&sets, &m](Hash setHash) {
    json::array_t children;
    m->visitMembers(
      [&children](Hash h) {
        children.push_back(h);
        return smtk::common::Visit::Continue;
      },
      setHash);
    sets[std::to_string(setHash)] = children;
    return smtk::common::Visit::Continue;
  });
  if (!sets.empty())
  {
    j["sets"] = sets;
  }
}

void from_json(const json& j, std::shared_ptr<Manager>& m)
{
  if (!m.get() || j.is_null())
  {
    m = smtk::string::Manager::create();
  }
  auto mit = j.find("members");
  if (mit != j.end())
  {
    std::unordered_map<Hash, std::string> members;
    std::unordered_map<Hash, std::unordered_set<Hash>> sets;
    for (const auto& element : mit->items())
    {
      members[element.value().get<Hash>()] = element.key();
    }
    auto sit = j.find("sets");
    if (sit != j.end())
    {
      for (const auto& element : sit->items())
      {
        Hash h = static_cast<Hash>(stoull(element.key(), nullptr));
        sets[h] = element.value().get<std::unordered_set<Hash>>();
      }
    }
    m->setData(members, sets);
  }
}

} // namespace string
} // namespace smtk

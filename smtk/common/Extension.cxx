//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Extension.h"

#include <map>

namespace smtk
{
namespace common
{

static std::map<std::string, std::pair<std::function<Extension::Ptr(void)>, bool>> s_extensionMap;

// std::map<std::string, std::pair<std::function<Extension::Ptr(void)>, bool>>
// Extension::s_extensionMap;

Extension::Extension() = default;

Extension::~Extension() = default;

bool Extension::registerExtension(
  const std::string& name,
  std::function<Extension::Ptr(void)> ctor,
  bool oneShot)
{
  if (name.empty() || !ctor)
  {
    return false;
  }
  auto it = s_extensionMap.find(name);
  if (it != s_extensionMap.end())
  {
    it->second.first = ctor;
    it->second.second = oneShot;
  }
  else
  {
    s_extensionMap.insert(std::make_pair(name, std::make_pair(ctor, oneShot)));
  }
  return true;
}

bool Extension::unregisterExtension(const std::string& name)
{
  auto it = s_extensionMap.find(name);
  if (it != s_extensionMap.end())
  {
    s_extensionMap.erase(it);
    return true;
  }
  return false;
}

void Extension::visitAll(
  std::function<std::pair<bool, bool>(const std::string&, Extension::Ptr)> visitor)
{
  auto it = s_extensionMap.begin();
  auto tmp = s_extensionMap.end();
  for (; it != s_extensionMap.end(); it = tmp)
  {
    std::pair<bool, bool> didUseAndTerminate = visitor(it->first, it->second.first());
    tmp = it;
    ++tmp;
    if (didUseAndTerminate.first)
    {
      if (it->second.second)
      {
        s_extensionMap.erase(it);
      }
    }
    if (didUseAndTerminate.second)
    {
      return;
    }
  }
}

Extension::Ptr Extension::find(const std::string& name, bool removeOneShot)
{
  Extension::Ptr result;
  auto it = s_extensionMap.find(name);
  if (it == s_extensionMap.end())
  {
    return result;
  }
  result = it->second.first();
  if (removeOneShot && it->second.second)
  {
    s_extensionMap.erase(it);
  }
  return result;
}
} // namespace common
} // namespace smtk

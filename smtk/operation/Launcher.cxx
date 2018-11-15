//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Launcher.h"

#include "smtk/io/Logger.h"

#include <thread>

namespace
{
// Key corresponding to the default operation launch method
smtk::operation::Launchers::LauncherMap::key_type default_key = "default";
}

namespace smtk
{
namespace operation
{

Launchers::Launchers()
{
  // The default launch method uses an asynchronous thread.
  this->insert(std::make_pair(default_key, [](const Operation::Ptr& op) {
    return std::async(std::launch::async, [&]() { return op->operate(); });
  }));
}

Launchers::Launchers(const LauncherMap::mapped_type& m_type)
{
  this->insert(std::make_pair(default_key, m_type));
}

std::pair<Launchers::LauncherMap::iterator, bool> Launchers::insert(
  const Launchers::LauncherMap::value_type& v_type)
{
  return m_launchers.insert(v_type);
}

std::pair<Launchers::LauncherMap::iterator, bool> Launchers::emplace(
  Launchers::LauncherMap::value_type&& v_type)
{
  return m_launchers.emplace(v_type);
}

Launchers::LauncherMap::mapped_type& Launchers::operator[](
  const Launchers::LauncherMap::key_type& k_type)
{
  return m_launchers[k_type];
}

Launchers::LauncherMap::size_type Launchers::erase(const Launchers::LauncherMap::key_type& k_type)
{
  if (k_type != default_key)
  {
    return m_launchers.erase(k_type);
  }

  return m_launchers.size();
}

std::future<Operation::Result> Launchers::operator()(
  const Operation::Ptr& op, const Launchers::LauncherMap::key_type& k_type)
{
  assert(op != nullptr);

  auto search = m_launchers.find(k_type);
  if (search != m_launchers.end())
  {
    return search->second(op);
  }
  else
  {
    smtkWarningMacro(op->log(), "Could not find operation launcher type \""
        << k_type << "\". Falling back to default operation launcher.");
    return this->operator()(op, "default");
  }
}

std::future<Operation::Result> Launchers::operator()(const Operation::Ptr& op)
{
  return this->operator()(op, default_key);
}

} // operation namespace
} // smtk namespace

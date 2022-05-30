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

#include "smtk/common/ThreadPool.h"

#include "smtk/io/Logger.h"

#include <functional>
#include <thread>

namespace
{
// Key corresponding to the default operation launch method
smtk::operation::Launchers::LauncherMap::key_type default_key = "default";

// The default launcher uses a thread pool and is copy-constructible (so it can
// be placed in a map).
class DefaultLauncher
{
public:
  DefaultLauncher()
    : m_pool(new smtk::common::ThreadPool<smtk::operation::Operation::Result>)
  {
  }

  DefaultLauncher(const DefaultLauncher& /*unused*/)
    : m_pool(new smtk::common::ThreadPool<smtk::operation::Operation::Result>)
  {
  }

  DefaultLauncher(DefaultLauncher&& other) noexcept { m_pool.swap(other.m_pool); }

  DefaultLauncher& operator=(DefaultLauncher&& other) noexcept
  {
    if (&other != this)
    {
      m_pool.swap(other.m_pool);
    }
    return *this;
  }

  std::shared_future<smtk::operation::Operation::Result> operator()(
    const smtk::operation::Operation::Ptr& operation)
  {
    return m_pool->operator()(
      [](const smtk::operation::Operation::Ptr& op) { return op->operate(); }, operation);
  }

private:
  std::unique_ptr<smtk::common::ThreadPool<smtk::operation::Operation::Result>> m_pool;
};
} // namespace

namespace smtk
{
namespace operation
{

Launchers::Launchers()
{
  m_launchers[default_key] = DefaultLauncher();
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

std::shared_future<Operation::Result> Launchers::operator()(
  const Operation::Ptr& op,
  const Launchers::LauncherMap::key_type& k_type)
{
  assert(op != nullptr);

  auto search = m_launchers.find(k_type);
  if (search != m_launchers.end())
  {
    return search->second(op);
  }
  else
  {
    smtkWarningMacro(
      op->log(),
      "Could not find operation launcher type \""
        << k_type << "\". Falling back to default operation launcher.");
    return this->operator()(op, "default");
  }
}

std::shared_future<Operation::Result> Launchers::operator()(const Operation::Ptr& op)
{
  return this->operator()(op, default_key);
}

} // namespace operation
} // namespace smtk

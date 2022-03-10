//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/json/Helper.h"

#include "smtk/io/Logger.h"

#include <thread>
#include <vector>

namespace
{
std::mutex g_types;
thread_local std::vector<std::unique_ptr<smtk::resource::json::Helper>> g_instanceStack;
} // anonymous namespace

namespace smtk
{
namespace resource
{
namespace json
{

Helper::Helper() = default;
Helper::~Helper() = default;

Helper& Helper::instance()
{
  if (g_instanceStack.empty())
  {
    g_instanceStack.emplace_back(new Helper);
  }
  return *(g_instanceStack.back());
}

Helper& Helper::pushInstance(smtk::resource::Resource* parent)
{
  (void)parent;
  std::shared_ptr<smtk::common::Managers> managers;
  if (!g_instanceStack.empty())
  {
    managers = g_instanceStack.back()->managers();
  }
  g_instanceStack.emplace_back(new Helper);
  g_instanceStack.back()->setManagers(managers);
  g_instanceStack.back()->m_topLevel = false;
  return *(g_instanceStack.back());
}

void Helper::popInstance()
{
  if (!g_instanceStack.empty())
  {
    g_instanceStack.pop_back();
  }
}

std::size_t Helper::nestingDepth()
{
  return g_instanceStack.size();
}

void Helper::setManagers(const smtk::common::Managers::Ptr& managers)
{
  m_managers = managers;
}

smtk::common::Managers::Ptr Helper::managers()
{
  return m_managers;
}

void Helper::clear()
{
  // Do nothing.
}

} // namespace json
} // namespace resource
} // namespace smtk

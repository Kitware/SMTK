//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Helper.h"

#include "smtk/io/Logger.h"

namespace
{
thread_local std::vector<std::unique_ptr<smtk::operation::Helper>> g_instanceStack;
} // anonymous namespace

namespace smtk
{
namespace operation
{

Helper::Helper() = default;
Helper::~Helper() = default;

Helper& Helper::instance()
{
  if (g_instanceStack.empty())
  {
    g_instanceStack.emplace_back(std::unique_ptr<Helper>(new Helper));
  }
  return *(g_instanceStack.back());
}

Helper& Helper::pushInstance(Operation::Key* opKey)
{
  g_instanceStack.emplace_back(new Helper);
  g_instanceStack.back()->m_key = opKey;
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

} // namespace operation
} // namespace smtk

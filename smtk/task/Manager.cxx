//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/Manager.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace task
{

constexpr const char* const Manager::type_name;

Manager::Manager()
  : m_taskInstances(*this)
  , m_active(&m_taskInstances)
{
}

Manager::Manager(smtk::resource::Resource* parent)
  : m_taskInstances(*this)
  , m_active(&m_taskInstances)
  , m_parent(parent)
{
}

Manager::~Manager() = default;

nlohmann::json Manager::getStyle(const smtk::string::Token& styleClass) const
{
  if (this->m_styles.contains(styleClass.data()))
  {
    return this->m_styles.at(styleClass.data());
  }
  return nlohmann::json();
}

smtk::resource::Resource* Manager::resource() const
{
  return m_parent;
}

} // namespace task
} // namespace smtk

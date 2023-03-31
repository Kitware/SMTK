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

Manager::~Manager() = default;

nlohmann::json Manager::getStyle(const smtk::string::Token& styleClass) const
{
  if (this->m_styles.contains(styleClass.data()))
  {
    return this->m_styles.at(styleClass.data());
  }
  return nlohmann::json();
}

} // namespace task
} // namespace smtk

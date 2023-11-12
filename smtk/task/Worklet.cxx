//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Worklet.h"
#include "smtk/task/Manager.h"

#include "smtk/string/json/jsonToken.h"
#include "smtk/task/json/Helper.h"

#include "smtk/io/Logger.h"

#include "smtk/common/UUIDGenerator.h"

#include <stdexcept>

namespace smtk
{
namespace task
{

Worklet::Worklet()
{
  m_id = smtk::common::UUIDGenerator::instance().random();
}

Worklet::Worklet(
  const Configuration& config,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  (void)managers;
  this->configure(config, taskManager);
}

void Worklet::configure(const Configuration& config, Manager& taskManager)
{
  if (!config.is_object())
  {
    throw std::logic_error("Invalid configuration passed to Worklet constructor.");
  }

  auto it = config.find("name");
  if (it == config.end())
  {
    throw std::logic_error(
      "Invalid configuration passed to Worklet constructor.  No name specified.");
  }
  m_name = it->get<std::string>();

  // Set the configuration and manager
  m_configuration = config;
  m_manager = taskManager.shared_from_this();

  it = config.find("id");
  if (it == config.end())
  {
    // The worklet's configuration has no UUID. Create one.
    this->setId(smtk::common::UUID::random());
  }
  else
  {
    m_id = it->get<smtk::common::UUID>();
    m_configuration["id"] = m_id.toString();
  }

  it = config.find("schema");
  if (it != config.end())
  {
    m_schema = it->get<std::string>();
  }
  else
  {
    m_schema.clear();
  }

  it = config.find("version");
  if (it != config.end())
  {
    m_version = *it;
  }
  else
  {
    m_version = 0;
    m_configuration["version"] = m_version;
  }

  it = config.find("operation");
  if (it != config.end())
  {
    m_operationName = it->get<std::string>();
  }
  else
  {
    m_operationName = "smtk::task::EmplaceWorklet";
    m_configuration["operation"] = "smtk::task::EmplaceWorklet";
  }

  it = config.find("description");
  if (it != config.end())
  {
    m_description = it->get<std::string>();
  }
  else
  {
    m_description.clear();
  }
}

const std::shared_ptr<resource::Resource> Worklet::resource() const
{
  std::shared_ptr<resource::Resource> rsrc;
  if (auto manager = m_manager.lock())
  {
    if (auto* ptr = manager->resource())
    {
      rsrc = ptr->shared_from_this();
    }
  }
  return rsrc;
}

Manager::Ptr Worklet::manager() const
{
  return m_manager.lock();
}

bool Worklet::setId(const common::UUID& newId)
{
  if (newId == m_id)
  {
    return false;
  }

  m_configuration["id"] = newId.toString();

  if (auto rsrc = this->resource())
  {
    // TODO: FIXME: ask resource to update our ID and index us.
  }
  m_id = newId;
  return true;
}

void Worklet::setName(const std::string& newName)
{
  m_name = newName;
  m_configuration["name"] = newName;
}

} // namespace task
} // namespace smtk

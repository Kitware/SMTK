//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Port.h"

#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"

#include "smtk/string/json/jsonToken.h"
#include "smtk/task/json/Helper.h"

#include "smtk/io/Logger.h"

#include "smtk/common/StringUtil.h"
#include "smtk/common/UUIDGenerator.h"

#include <stdexcept>

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{

// constexpr const char* const Port::type_name;

Port::Port()
  : m_id(smtk::common::UUIDGenerator::instance().random())
{
}

Port::Port(
  const Configuration& config,
  Task* parentTask,
  const std::shared_ptr<smtk::common::Managers>& managers)
  : m_id(smtk::common::UUIDGenerator::instance().random())
  , m_parent(parentTask)
  , m_manager(parentTask->manager()->shared_from_this())
{
  (void)managers;
  this->configure(config);
}

Port::Port(
  const Configuration& config,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
  : m_id(smtk::common::UUIDGenerator::instance().random())
  , m_manager(taskManager.shared_from_this())
{
  (void)managers;
  this->configure(config);
}

void Port::configure(const Configuration& config)
{
  if (!config.is_object())
  {
    throw std::logic_error("Invalid configuration data passed to Port constructor.");
  }
  auto it = config.find("id");
  if (it == config.end() || it->is_number_integer())
  {
    // We are deserializing a task "template" which has no UUID. Make one up.
    this->setId(smtk::common::UUID::random());
  }
  else
  {
    this->setId(it->get<smtk::common::UUID>());
  }
  it = config.find("direction");
  if (it != config.end())
  {
    m_direction = Port::DirectionFromLabel(it->get<std::string>());
  }
  else
  {
    m_direction = Direction::In; // TODO: Warn?
  }

  it = config.find("access");
  if (it != config.end())
  {
    m_access = Port::AccessFromLabel(it->get<std::string>());
  }
  else
  {
    m_access = Access::External; // TODO: Warn?
  }

  it = config.find("name");
  if (it != config.end())
  {
    this->setName(it->get<std::string>());
  }

  it = config.find("data-types");
  if (it != config.end())
  {
    try
    {
      m_dataTypes = it->get<std::unordered_set<smtk::string::Token>>();
    }
    catch (nlohmann::json::exception&)
    {
    }
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Port configuration must include data-types.");
  }

  it = config.find("style");
  if (it != config.end())
  {
    try
    {
      m_style = it->get<std::unordered_set<smtk::string::Token>>();
    }
    catch (nlohmann::json::exception&)
    {
    }
  }
  // NB: Do not deserialize connections here as tasks are not created
  //     until after ports during project::Read. Instead, we provide
  //     a way for connections to be assigned directly.
  // NB: Do not deserialize parent here as tasks are not created
  //     until after ports during project::Read.
}

bool Port::setId(const common::UUID& newId)
{
  auto fp = [this, newId]() {
    if (this->m_id == newId)
    {
      return false;
    }
    this->m_id = newId;
    return true;
  };

  if (auto manager = m_manager.lock())
  {
    return manager->changePortId(this, fp);
  }
  return fp();
}

bool Port::setName(const std::string& newName)
{
  auto fp = [this, newName]() {
    if (this->m_name == newName)
    {
      return false;
    }
    this->m_name = newName;
    return true;
  };

  if (m_parent)
  {
    return m_parent->changePortName(this, newName, fp);
  }
  return fp();
}

std::shared_ptr<PortData> Port::portData(smtk::resource::PersistentObject* object) const
{
  if (auto* otherPort = dynamic_cast<Port*>(object))
  {
    if (auto* otherTask = otherPort->parent())
    {
      return otherTask->portData(otherPort);
    }
  }
  else
  {
    if (m_dataTypes.find(smtk::common::typeName<ObjectsInRoles>()) != m_dataTypes.end())
    {
      auto data = ObjectsInRoles::create();
      data->addObject(object, "unassigned");
      return data;
    }
  }
  return nullptr;
}

const std::shared_ptr<resource::Resource> Port::resource() const
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

bool Port::addStyle(const smtk::string::Token& styleClass)
{
  if (styleClass.id() == smtk::string::Manager::Invalid)
  {
    return false;
  }
  return m_style.insert(styleClass).second;
}

bool Port::removeStyle(const smtk::string::Token& styleClass)
{
  return m_style.erase(styleClass) > 0;
}

bool Port::clearStyle()
{
  bool didModify = !m_style.empty();
  m_style.clear();
  return didModify;
}

bool Port::getViewData(smtk::common::TypeContainer& configuration) const
{
  (void)configuration;
  return false;
}

bool Port::setParent(Task* parent)
{
  if (m_parent == parent)
  {
    return false;
  }
  m_parent = parent;
  return true;
}

Port::Direction Port::DirectionFromLabel(const std::string& label)
{
  std::string labelCopy(label);
  smtk::string::Token down(smtk::common::StringUtil::lower(labelCopy));
  switch (down.id())
  {
    case "smtk::task::port::direction::out"_hash:
    case "port::direction::out"_hash:
    case "direction::out"_hash:
    case "out"_hash:
      return Direction::Out;
      break;

    default:
    case "smtk::task::port::direction::in"_hash:
    case "port::direction::in"_hash:
    case "direction::in"_hash:
    case "in"_hash:
      return Direction::In;
      break;
  }
}

Port::Access Port::AccessFromLabel(const std::string& label)
{
  std::string labelCopy(label);
  smtk::string::Token down(smtk::common::StringUtil::lower(labelCopy));
  switch (down.id())
  {
    case "smtk::task::access::external"_hash:
    case "port::access::external"_hash:
    case "access::external"_hash:
    case "external"_hash:
      return Access::External;
      break;

    default:
    case "smtk::task::port::access::internal"_hash:
    case "port::access::internal"_hash:
    case "access::internal"_hash:
    case "internal"_hash:
      return Access::Internal;
      break;
  }
}

smtk::string::Token Port::LabelFromDirection(Direction dir)
{
  switch (dir)
  {
    case Out:
      return "out";
    default:
    case In:
      return "in";
  }
}

smtk::string::Token Port::LabelFromAccess(Access a)
{
  switch (a)
  {
    case External:
      return "external";
    default:
    case Internal:
      return "internal";
  }
}

} // namespace task
} // namespace smtk

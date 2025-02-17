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

#include "smtk/resource/Manager.h"

#include "smtk/io/Logger.h"

#include "smtk/common/StringUtil.h"
#include "smtk/common/UUIDGenerator.h"

#include <stdexcept>

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{
namespace
{

std::string objTypeName(
  smtk::resource::PersistentObject* obj,
  std::unordered_map<smtk::string::Token, smtk::string::Token>* typeLabels)
{
  smtk::string::Token tn = obj->typeName();
  if (typeLabels)
  {
    auto it = typeLabels->find(tn);
    if (it != typeLabels->end())
    {
      tn = it->second;
    }
  }
  return tn.data();
}

void summarizePortData(
  std::ostringstream& ttip,
  const std::shared_ptr<smtk::task::ObjectsInRoles>& pdata,
  std::unordered_map<smtk::string::Token, smtk::string::Token>* typeLabels)
{
  if (!pdata || pdata->data().empty())
  {
    return;
  }

  ttip << "<ul>\n";
  for (const auto& roleEntry : pdata->data())
  {
    ttip << "  <li><i>" << roleEntry.first.data() << "</i>";
    if (!roleEntry.second.empty())
    {
      ttip << "\n  <ul>\n";
      for (const auto& obj : roleEntry.second)
      {
        ttip << "    <li><b>" << obj->name() << "</b> (" << objTypeName(obj, typeLabels) << ")";
        if (auto* port = dynamic_cast<smtk::task::Port*>(obj))
        {
          ttip << " from " << port->parent()->name();
        }
        ttip << "</li>\n";
      }
      ttip << "  </ul>\n";
    }
    ttip << "</li>\n";
  }
  ttip << "</ul>\n";
}

} // anonymous namespace

// constexpr const char* const Port::type_name;

Port::Port()
  : m_id(smtk::common::UUIDGenerator::instance().random())
  , m_unassignedRole("unassigned"_token)
{
}

Port::Port(
  const Configuration& config,
  Task* parentTask,
  const std::shared_ptr<smtk::common::Managers>& managers)
  : m_id(smtk::common::UUIDGenerator::instance().random())
  , m_parent(parentTask)
  , m_manager(parentTask->manager()->shared_from_this())
  , m_unassignedRole("unassigned"_token)
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
  , m_unassignedRole("unassigned"_token)
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

  it = config.find("unassigned-role");
  m_unassignedRole = (it != config.end() ? it->get<smtk::string::Token>() : "unassigned"_token);

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
  // We deserialize connections here, so that ports may be
  // completely configured from JSON data. However, this
  // initialization may be partial when loading a task::Manager
  // from disk as tasks are not created until after ports
  // during project::Read. Port::configureConnections() is
  // a separate method to connections to be assigned directly
  // by operations such as this.
  it = config.find("connections");
  if (it != config.end())
  {
    this->configureConnections(*it);
  }
  // We deserialize parent here if possible, but be aware that
  // tasks are not created until after ports by the task::Manager's
  // JSON deserializer.
  it = config.find("parent");
  if (it != config.end())
  {
    auto& helper = json::Helper::instance();
    m_parent = helper.objectFromJSONSpecAs<Task>(*it);
  }
}

void Port::configureConnections(const Configuration& connConfig)
{
  auto& helper = json::Helper::instance();
  for (const auto& connSpec : connConfig)
  {
    if (auto* obj = helper.objectFromJSONSpec(connSpec))
    {
      m_connections.insert(obj);
    }
  }
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
      data->addObject(object, m_unassignedRole);
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

std::string Port::describe() const
{
  auto* rsrc = this->parentResource();
  auto rsrcMgr = rsrc ? rsrc->manager() : nullptr;
  auto* typeLabels = rsrcMgr ? &rsrcMgr->objectTypeLabels() : nullptr;

  std::ostringstream ttip;
  ttip << "<html><body>\n<h1>" << this->name() << "</h1>\n";
  if (this->direction() == smtk::task::Port::Direction::In && !this->connections().empty())
  {
    ttip << "<ul>\n";
    for (const auto& obj : this->connections())
    {
      if (obj)
      {
        ttip << "  <li><b>" << obj->name() << "</b> (" << objTypeName(obj, typeLabels) << ")";
        if (auto* port = dynamic_cast<smtk::task::Port*>(obj))
        {
          ttip << " from " << port->parent()->name();
          auto pdata = std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(this->portData(port));
          summarizePortData(ttip, pdata, typeLabels);
        }
        else
        {
          ttip << " as <i>" << this->unassignedRole().data() << "</i>";
        }
        ttip << "</li>\n";
      }
    }
    ttip << "</ul>\n";
  }
  else if (this->direction() == smtk::task::Port::Direction::Out)
  {
    auto pdata =
      std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(this->parent()->portData(this));
    summarizePortData(ttip, pdata, typeLabels);
  }
  ttip << "</body></html>";
  return ttip.str();
}

} // namespace task
} // namespace smtk

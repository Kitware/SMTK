//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.cxx - Abstract base class for CMB resources
// .SECTION Description
// .SECTION See Also

#include "smtk/resource/Resource.h"

#include "smtk/resource/CopyOptions.h"
#include "smtk/resource/Manager.h"

#include "smtk/resource/filter/Filter.h"
#include "smtk/resource/filter/ResourceActions.h"

#include "smtk/common/Paths.h"
#include "smtk/common/TypeName.h"
#include "smtk/common/UUIDGenerator.h"

#include "smtk/io/Logger.h"

#include "units/System.h"
#include "units/json/jsonUnits.h"

namespace smtk
{
namespace resource
{

constexpr smtk::resource::Links::RoleType Resource::VisuallyLinkedRole;
constexpr const char* const Resource::type_name;
const Resource::Index Resource::type_index = std::type_index(typeid(Resource)).hash_code();

Resource::Resource(const smtk::common::UUID& myID, ManagerPtr manager)
  : m_manager(manager)
  , m_id(myID)
  , m_clean(false)
  , m_links(this)
  , m_properties(this)
{
}

Resource::Resource(ManagerPtr manager)
  : Resource(smtk::common::UUIDGenerator::instance().random(), manager)
{
}

Resource::Resource(Resource&& rhs) noexcept
  : m_manager(std::move(rhs.m_manager))
  , m_id(std::move(rhs.m_id))
  , m_location(std::move(rhs.m_location))
  , m_name(std::move(rhs.m_name))
  , m_clean(std::move(rhs.m_clean))
  , m_links(std::move(rhs.m_links))
  , m_properties(std::move(rhs.m_properties))
  , m_queries(std::move(rhs.m_queries))
{
}

Resource::~Resource()
{
  // The normal way resources are removed (when they are held by a
  // manager) is for an operation to insert the resource into its
  // result's "resourcesToExpunge" item, which holds a shared reference
  // to it. Since the result is returned to the caller of the operation's
  // operate() method after the lock has been freed, this should be the
  // last shared pointer referencing the resource and no lock should be
  // held for the resource when the result goes out of scope.
  //
  // Thus, if you see this error, it is likely you are trying to remove
  // the resource from a resource::Manager yourself – rather than letting
  // the base Operation class do it for your – or are dropping
  // shared ownership of an unmanaged resource while it is locked (which
  // you should avoid by releasing the lock before dropping the shared
  // pointer).
  //
  // Why this is important: if a resource is destroyed while the lock is
  // held, subtle errors may accumulate making it difficult to identify
  // the point when using the resource became unsafe. This helps by
  // warning at a point when using the resource becomes unsafe.
  if (m_lock.state() != LockType::Unlocked)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Deleting locked resource " << this << " " << this->id()
                                  << ". Perhaps your operation needs to report this resource "
                                     "in the \"resourcesToExpunge\" item so it outlives lock "
                                     "removal?");
  }
}

Component* Resource::component(const smtk::common::UUID& compId) const
{
  return this->find(compId).get();
}

std::function<bool(const Component&)> Resource::queryOperation(
  const std::string& filterString) const
{
  // By default, return a filter that discriminates according to the default
  // property types.
  return smtk::resource::filter::Filter<>(filterString);
}

ComponentSet Resource::filter(const std::string& queryString) const
{
  // Construct a query operation from the query string
  auto queryOp = this->queryOperation(queryString);

  // Construct a component set to fill
  ComponentSet componentSet;

  // Visit each component and add it to the set if it satisfies the query
  smtk::resource::Component::Visitor visitor = [&](const ComponentPtr& component) {
    if (queryOp(*component))
    {
      componentSet.insert(component);
    }
  };

  this->visit(visitor);

  return componentSet;
}

bool Resource::isOfType(const Resource::Index& index) const
{
  return this->index() == index;
}

bool Resource::isOfType(const std::string& typeName) const
{
  return smtk::common::typeName<Resource>() == typeName;
}

int Resource::numberOfGenerationsFromBase(const std::string& typeName) const
{
  return (typeName == smtk::common::typeName<Resource>() ? 0 : std::numeric_limits<int>::lowest());
}

bool Resource::setId(const smtk::common::UUID& myId)
{
  if (myId == m_id)
  {
    return false;
  }

  Manager::Ptr mgr = m_manager.lock();
  if (mgr)
  {
    // If the resource's manager is set, then the resource is registered to a
    // manager. We need to change the resource's Id while ensuring we do not
    // break the indexing of the manager's collection of resources.
    common::UUID originalId(m_id);
    mgr->reviseId(SetId(originalId), SetId(myId));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<smtk::common::UUID*>(&m_id) = myId;
  }

  if (myId == m_id)
  {
    this->setClean(false);
    return true;
  }
  return false;
}

bool Resource::setLocation(const std::string& myLocation)
{
  if (myLocation == m_location)
  {
    return false;
  }

  Manager::Ptr mgr = m_manager.lock();
  if (mgr)
  {
    std::string originalLocation(m_location);
    mgr->reviseLocation(m_id, SetLocation(originalLocation), SetLocation(myLocation));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<std::string*>(&m_location) = myLocation;
  }

  if (myLocation == m_location)
  {
    this->setClean(false);
    return true;
  }
  return false;
}

std::string Resource::name() const
{
  if (m_name.empty())
  {
    return smtk::common::Paths::stem(m_location);
  }
  return m_name;
}

bool Resource::setName(const std::string& name)
{
  if (name == m_name)
  {
    return false;
  }
  m_name = name;
  this->setClean(false);
  return true;
}

void Resource::setClean(bool state)
{
  m_clean = state;
}

bool Resource::setUnitsSystem(const shared_ptr<units::System>& unitsSystem)
{
  m_unitsSystem = unitsSystem;
  return true;
}

bool Resource::setTemplateType(const smtk::string::Token& templateType)
{
  (void)templateType;
  return false;
}

smtk::string::Token Resource::templateType() const
{
  static smtk::string::Token empty;
  return empty;
}

bool Resource::setTemplateVersion(std::size_t templateVersion)
{
  (void)templateVersion;
  return false;
}

std::size_t Resource::templateVersion() const
{
  return 0;
}

std::shared_ptr<Resource> Resource::clone(CopyOptions& options) const
{
  (void)options;
  std::shared_ptr<Resource> nil;
  return nil;
}

bool Resource::copyInitialize(const std::shared_ptr<const Resource>& source, CopyOptions& options)
{
  (void)source;
  (void)options;
  return false;
}

bool Resource::copyFinalize(const std::shared_ptr<const Resource>& source, CopyOptions& options)
{
  (void)source;
  (void)options;
  return false;
}

void Resource::copyUnitSystem(
  const std::shared_ptr<const Resource>& rsrc,
  const CopyOptions& options)
{
  switch (options.copyUnitSystem())
  {
    case CopyOptions::CopyType::None:
      // Do not set a unit system.
      break;
    case CopyOptions::CopyType::Shallow:
      this->setUnitsSystem(rsrc->unitsSystem());
      break;
    case CopyOptions::CopyType::Deep:
    {
      if (auto unitSys = rsrc->unitsSystem())
      {
        nlohmann::json spec = unitSys;
        shared_ptr<units::System> unitCopy = units::System::createFromSpec(spec.dump());
        this->setUnitsSystem(unitCopy);
      }
    }
    break;
  }
}

void Resource::copyLinks(const std::shared_ptr<const Resource>& rsrc, const CopyOptions& options)
{
  this->links().copyFrom(rsrc, options);
}

void Resource::copyProperties(const std::shared_ptr<const Resource>& rsrc, CopyOptions& options)
{
  if (!options.copyProperties())
  {
    return;
  }

  // In order to obey options.copyComponents(), we create/populate a
  // list of UUIDs to skip when not copying components.
  if (!options.copyComponents())
  {
    options.omitComponents(rsrc);
  }
  const auto& sourceMap = rsrc->properties().data().data();
  auto& targetMap = this->properties().data().data();
  std::size_t numCopied = 0;
  for (const auto& sourceEntry : sourceMap)
  {
    auto it = targetMap.find(sourceEntry.first);
    if (it == targetMap.end())
    {
      smtkErrorMacro(options.log(), "No matching \"" << sourceEntry.first << "\" property data.");
      continue;
    }
    auto* targetProps = dynamic_cast<detail::PropertiesBase*>(it->second);
    auto* sourceProps = dynamic_cast<detail::PropertiesBase*>(sourceEntry.second);
    if (!targetProps || !sourceProps)
    {
      smtkErrorMacro(
        options.log(),
        "The \"" << sourceEntry.first << "\" property data cannot be cast to the same type.");
      continue;
    }
    smtk::string::Token invalid;
    std::set<smtk::common::UUID> sourceIds;
    sourceProps->allIds(sourceIds);
    for (const auto& sourceId : sourceIds)
    {
      // Skip component properties when not copying components:
      if (options.shouldOmitId(sourceId))
      {
        continue;
      }

      smtk::common::UUID targetId;
      if (auto* targetObj = options.targetObjectFromSourceId<PersistentObject>(sourceId))
      {
        targetId = targetObj->id();
      }
      else
      {
        targetId = sourceId;
      }
      numCopied += targetProps->copyFrom(sourceProps, invalid, sourceId, targetId);
    }
  }
  smtkInfoMacro(options.log(), "Copied " << numCopied << " properties\n");

  // The problem with the approach below is inefficiency; it will visit every property type
  // and every property name for every UUID.
#if 0
  // Copy the resource's properties:
  this->copyPropertiesForId(rsrc, rsrc->id(), this->id(), options);

  // Copy each component's properties:
  if (options.copyComponents())
  {
    // Visitor to copy properties of each component.
    ComponentVisitor copyComponentProperties = [&](const Component& source)
    {
      auto* target = options.targetObjectFromSourceId<Component>(source.id());
      if (target)
      {
        this->copyPropertiesForId(rsrc, source.id(), target->id(), options);
      }
      else
      {
        smtkErrorMacro(options.log(),
          "No mapping from component \"" << source.name() << "\" (" << source.id() << ")"
          " so no properties copied.");
      }
    };

    rsrc->visit(copyComponentProperties);
  }
#endif
}

bool Resource::copyPropertiesForId(
  const std::shared_ptr<Resource>& rsrc,
  const smtk::common::UUID& sourceId,
  const smtk::common::UUID& targetId,
  const CopyOptions& options)
{
  (void)rsrc;
  (void)sourceId;
  (void)targetId;
  (void)options;
  return false;
}

bool Resource::setManager(ManagerPtr newManager)
{
  if (m_manager.lock())
  {
    return false;
  }
  m_manager = newManager;
  return true;
}

} // namespace resource
} // namespace smtk

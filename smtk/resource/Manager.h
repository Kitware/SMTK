//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Manager_h
#define smtk_resource_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/Processing.h"
#include "smtk/common/TypeName.h"
#include "smtk/common/UUID.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Lock.h"
#include "smtk/resource/Metadata.h"
#include "smtk/resource/MetadataContainer.h"
#include "smtk/resource/Observer.h"
#include "smtk/resource/Resource.h"

#include <functional>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace smtk
{
namespace resource
{
class GarbageCollector;
class Manager;
using GarbageCollectorPtr = std::shared_ptr<GarbageCollector>;

/// A resource Manager is responsible for tracking currently allocated
/// resources, creating new resources and serializing/deserializing resources
/// to/from disk. Resource types must first be registered with the Manager
/// before resources of they can be manipulated by the manager.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  /// The signature for visitor functions used to traverse managed resources.
  using ResourceVisitor = std::function<smtk::common::Processing(Resource&)>;

  smtkTypedefs(smtk::resource::Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Register a resource identified by its class type, read and write
  /// operations.
  template<typename ResourceType>
  bool registerResource(
    const std::function<
      ResourcePtr(const std::string&, const std::shared_ptr<smtk::common::Managers>&)>& read =
      nullptr,
    const std::function<bool(const ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)>&
      write = nullptr,
    const std::function<
      ResourcePtr(const smtk::common::UUID&, const std::shared_ptr<smtk::common::Managers>&)>&
      create = nullptr);

  /// Register a resource identified by its metadata.
  bool registerResource(Metadata&&);

  /// Unregister a resource identified by its class type.
  template<typename ResourceType>
  bool unregisterResource();

  /// Unregister a resource identified by its type name.
  bool unregisterResource(const std::string&);

  /// Unregister a resource identified by its type index.
  bool unregisterResource(const Resource::Index&);

  /// Check if a resource identified by its class type is registered.
  template<typename ResourceType>
  bool registered() const;

  /// Check if a resource idenfified by its type name is registered.
  bool registered(const std::string&) const;

  /// Check if a resource identified by its type index is registered.
  bool registered(const Resource::Index&) const;

  /// Remove all resources. This doesn't explicitly release the memory of the
  /// resources, it only stops the tracking of the resources by the manager.
  void clear();

  /// Construct a resource identified by its type name.
  ResourcePtr create(const std::string&, const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Construct a resource identified by its type index.
  ResourcePtr create(
    const Resource::Index&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Construct a resource identified by its class type.
  template<typename ResourceType>
  smtk::shared_ptr<ResourceType> create(const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Construct a resource with a given UUID identified by its type name.
  ResourcePtr create(
    const std::string&,
    const smtk::common::UUID&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Construct a resource with a given UUID identified by its type index.
  ResourcePtr create(
    const Resource::Index&,
    const smtk::common::UUID&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Construct a resource with a given UUID identified by its class type.
  template<typename ResourceType>
  smtk::shared_ptr<ResourceType> create(
    const smtk::common::UUID&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Returns the resource that relates to the given uuid.  If no association exists
  /// this will return a null pointer
  ResourcePtr get(const smtk::common::UUID& id);
  ConstResourcePtr get(const smtk::common::UUID& id) const;

  /// Returns the resource that relates to the given uuid.  If no association
  /// exists of this type, this will return a null pointer.
  template<typename ResourceType>
  smtk::shared_ptr<ResourceType> get(const smtk::common::UUID&);
  template<typename ResourceType>
  smtk::shared_ptr<const ResourceType> get(const smtk::common::UUID&) const;

  /// Returns the resource that relates to the given \a url.  If no association exists
  /// this will return a null pointer
  ResourcePtr get(const std::string& url);
  ConstResourcePtr get(const std::string& url) const;

  /// Returns the resource that relates to the given \a url.  If no association
  /// exists of this type, this will return a null pointer.
  template<typename ResourceType>
  smtk::shared_ptr<ResourceType> get(const std::string& url);
  template<typename ResourceType>
  smtk::shared_ptr<const ResourceType> get(const std::string& url) const;

  /// Returns a set of resources that have a given type name.
  std::set<ResourcePtr> find(const std::string& typeName);

  /// Returns a set of resources that have a given type index..
  ///
  /// If \a strict is true, only resource with the exact \a typeIndex
  /// (instead of resources that also inherit \a typeIndex) will be
  /// returned.
  std::set<ResourcePtr> find(const Resource::Index& typeIndex, bool strict = false);

  /// Returns a set of resources that are of the type \a ResourceType.
  template<typename ResourceType>
  std::set<smtk::shared_ptr<ResourceType>> find();

  /// Read resource identified by its type index from file.
  ResourcePtr read(
    const std::string&,
    const std::string&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Read resource identified by its type index from file.
  ResourcePtr read(
    const Resource::Index&,
    const std::string&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Read resource from file.
  template<typename ResourceType>
  smtk::shared_ptr<ResourceType> read(
    const std::string&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Write resource to file. The resource's write location is held by the
  /// resource itself.
  bool write(const ResourcePtr&, const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Write resource to file. The resource's write location is passed as an
  /// input parameter.
  bool write(
    const ResourcePtr&,
    const std::string&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Add a resource identified by its type index. Returns true if the resource
  /// was added or already is part of this manager. If the resource is currently
  /// part of a different manager, we will reparent it to this manager.
  bool add(const Resource::Index&, const ResourcePtr&);

  /// Add a resource identified by its class type. Returns true if the resource
  /// was added or already is part of this manager.
  template<typename ResourceType>
  bool add(const smtk::shared_ptr<ResourceType>&);

  /// Returns true if the resource was added or already is part of this manager.
  bool add(const ResourcePtr&);

  /// Removes a resource from a given Manager. This doesn't explicitly release
  /// the memory of the resource, it only stops the tracking of the resource
  /// by the manager.
  bool remove(const ResourcePtr&);

  /// To maintain backwards compatibility, this method provides a means of
  /// registering an alias, or additional type name, to a resource type.
  /// There can be multiple aliases for each resource type, but an alias can
  /// only refer to a single resource type.
  bool addLegacyReader(const std::string&, const std::function<ResourcePtr(const std::string&)>&);

  /// Change the ID of a managed resource.
  ///
  /// This method cannot be called directly (only Resource can create its arguments).
  /// Instead, call Resource::setId(), which will invoke this method if the
  /// resource has a manager set.
  void reviseId(const Resource::SetId& source, const Resource::SetId& destination);

  /// Change the location of a managed resource.
  ///
  /// This method cannot be called directly (only Resource can create its arguments).
  /// Instead, call Resource::setLocation(), which will invoke this method if the
  /// resource has a manager set.
  void reviseLocation(
    const smtk::common::UUID& uid,
    const Resource::SetLocation& source,
    const Resource::SetLocation& destination);

  /// Visit resources held by this manager.
  ///
  /// This method read-locks the resource manager container.
  /// This means that visitors may query the resource manager but they may
  /// not alter the state of the container. If that must happen, changes should
  /// be queued on a captured variable and performed after this function returns.
  ///
  /// This method returns Termination::NORMAL when all resources were traversed
  /// and Termination::EARLY when a visitor halted iteration.
  smtk::common::Termination visit(const ResourceVisitor& visitor) const;

  /// Does the manager hold any resources?
  bool empty() const
  {
    ScopedLockGuard guard(m_lock, LockType::Read);
    return m_resources.empty();
  }

  /// Return the number of resources held by the manager.
  ///
  /// Note that because this number may be changed by other threads even before this
  /// method returns, it is only intended as a convenience for single-threaded tests.
  std::size_t size() const
  {
    ScopedLockGuard guard(m_lock, LockType::Read);
    return m_resources.size();
  }

  /// Return the map of metadata.
  MetadataContainer& metadata() { return m_metadata; }

  /// Return the observers associated with this manager.
  ///
  /// Note that observers may query but not modify the resource manager
  /// since it will be read-locked by the method that triggers the observers.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Return the metadata observers associated with this manager.
  Metadata::Observers& metadataObservers() { return m_metadataObservers; }
  const Metadata::Observers& metadataObservers() const { return m_metadataObservers; }

  /// Return a garbage collector used to clean up ephemeral objects after their use.
  GarbageCollectorPtr garbageCollector() { return m_garbageCollector; }

private:
  Manager();

  /// All resources are tracked using a map between the resource's UUID and a
  /// shared pointer to the resource itself.
  Container m_resources;

  /// A container for all registered resource metadata.
  MetadataContainer m_metadata;

  /// A container for all resource observers.
  Observers m_observers;

  /// A container for all resource metadata observers.
  Metadata::Observers m_metadataObservers;

  /// A map connecting legacy resource names to legacy readers.
  std::map<std::string, std::function<ResourcePtr(const std::string&)>> m_legacyReaders;

  /// A set of operations to delete ephemeral objects.
  GarbageCollectorPtr m_garbageCollector;

  /// A read/write-lock for controlling access to m_resources.
  mutable Lock m_lock;
};

template<typename ResourceType>
bool Manager::unregisterResource()
{
  return this->unregisterResource(std::type_index(typeid(ResourceType)).hash_code());
}

template<typename ResourceType>
bool Manager::registered() const
{
  return this->registered(std::type_index(typeid(ResourceType)).hash_code());
}

template<typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::create(
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  return smtk::static_pointer_cast<ResourceType>(
    this->create(std::type_index(typeid(ResourceType)).hash_code(), managers));
}

template<typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::create(
  const smtk::common::UUID& id,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  return smtk::static_pointer_cast<ResourceType>(
    this->create(std::type_index(typeid(ResourceType)).hash_code(), id, managers));
}

template<typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::get(const smtk::common::UUID& id)
{
  return smtk::static_pointer_cast<ResourceType>(this->get(id));
}

template<typename ResourceType>
smtk::shared_ptr<const ResourceType> Manager::get(const smtk::common::UUID& id) const
{
  return smtk::static_pointer_cast<const ResourceType>(this->get(id));
}

template<typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::get(const std::string& url)
{
  return smtk::static_pointer_cast<ResourceType>(this->get(url));
}

template<typename ResourceType>
smtk::shared_ptr<const ResourceType> Manager::get(const std::string& url) const
{
  return smtk::static_pointer_cast<const ResourceType>(this->get(url));
}

template<typename ResourceType>
std::set<smtk::shared_ptr<ResourceType>> Manager::find()
{
  Resource::Index index(typeid(ResourceType).hash_code());
  std::set<Resource::Index> validIndices;
  for (const auto& metadatum : m_metadata)
  {
    if (metadatum.m_parentIndices.find(index) != metadatum.m_parentIndices.end())
    {
      validIndices.insert(metadatum.index());
    }
  }

  std::set<smtk::shared_ptr<ResourceType>> values;

  {
    ScopedLockGuard guard(m_lock, LockType::Read);
    typedef Container::index<IndexTag>::type ResourcesByIndex;
    ResourcesByIndex& resources = m_resources.get<IndexTag>();
    for (const auto& idx : validIndices)
    {
      auto resourceItRange = resources.equal_range(idx);
      for (auto& it = resourceItRange.first; it != resourceItRange.second; ++it)
      {
        values.insert(smtk::static_pointer_cast<ResourceType>(*it));
      }
    }
  }

  return values;
}

template<typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::read(
  const std::string& url,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  return smtk::static_pointer_cast<ResourceType>(
    this->read(std::type_index(typeid(ResourceType)).hash_code(), url, managers));
}

template<typename ResourceType>
bool Manager::add(const smtk::shared_ptr<ResourceType>& resource)
{
  return this->add(std::type_index(typeid(ResourceType)).hash_code(), resource);
}

namespace detail
{
// For Resources that are derived from other Resources, we need a means to
// extract the Indices for both the Resource and its parent Resources. This
// information is subsequently used for retrieving derived Resources from
// queries made for parent Resources. We accomplish this by requiring that
// derived Resources define a ParentResource that relates to its parent
// resource. The chain of derived Resources is then recursively iterated, and
// each parent Resource::Index is added to the Resource Metadata's associated
// indices.

// A compile-time test to check whether or not a class has a ParentResource
// defined.
template<typename T>
class is_derived_resource
{
  class No
  {
  };
  class Yes
  {
    No no[2];
  };

  template<typename C>
  static Yes Test(typename C::ParentResource*);
  template<typename C>
  static No Test(...);

public:
  enum
  {
    value = sizeof(Test<T>(nullptr)) == sizeof(Yes)
  };
};

// The signature for our index generator has two template parameters.
template<typename ResourceType, bool derived_resource>
struct resource_index_set_generator;

// This partial template specialization deals with the case where
// <ResourceType> is not derived from another Resource. In this case, only the
// indices for the Resource and smtk::resource::Resource are added to the set of
// associated indices.
template<typename ResourceType>
struct resource_index_set_generator<ResourceType, false>
{
  static std::set<Resource::Index> indices()
  {
    std::set<Resource::Index> indices;
    indices.insert(typeid(ResourceType).hash_code());
    indices.insert(smtk::resource::Resource::type_index);

    return indices;
  }
};

// This partial template specialization deals with the case where
// <ResourceType> is derived from another Resource. In this case, we
// recursively add the parent Resource's associated indices to the returned
// set.
template<typename ResourceType>
struct resource_index_set_generator<ResourceType, true>
{
  static std::set<Resource::Index> indices()
  {
    std::set<Resource::Index> indices;
    indices.insert(std::type_index(typeid(ResourceType)).hash_code());

    std::set<Resource::Index> parentIndices = resource_index_set_generator<
      typename ResourceType::ParentResource,
      is_derived_resource<typename ResourceType::ParentResource>::value>::indices();
    indices.insert(parentIndices.begin(), parentIndices.end());

    return indices;
  }
};
} // namespace detail

template<typename ResourceType>
bool Manager::registerResource(
  const std::function<
    ResourcePtr(const std::string&, const std::shared_ptr<smtk::common::Managers>&)>& read,
  const std::function<bool(const ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)>&
    write,
  const std::function<
    ResourcePtr(const smtk::common::UUID&, const std::shared_ptr<smtk::common::Managers>&)>& create)
{
  // For standard Resources, the metadata is comprised of the following:
  // Type Name: either the "type_name" field or (if the former does not exist)
  //            the typeName() value
  // Index: the hash of the type_index
  // Parent Indices: a set of indices constructed by traversing parent resources
  //                 (defined using the typedef "ParentResource")
  // Create Functor: either a user-defined functor or the default
  //                 ResourceType::create method
  // Read Functor:   either a user-defined functor or the nullptr
  // Write Functor:   either a user-defined functor or the nullptr

  return Manager::registerResource(Metadata(
    smtk::common::typeName<ResourceType>(), std::type_index(typeid(ResourceType)).hash_code(),
    (detail::resource_index_set_generator<ResourceType,
      detail::is_derived_resource<ResourceType>::value>::indices()),
    (create ? create : [](const smtk::common::UUID& id, const std::shared_ptr<smtk::common::Managers>&) {
      Resource::Ptr resource = ResourceType::create();
      resource->setId(id);
      return resource;
    }),
    (read ? read : [](const std::string&, const std::shared_ptr<smtk::common::Managers>&){ return ResourcePtr(); }),
    (write ? write : [](const ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&){ return false; })));
}
} // namespace resource
} // namespace smtk

#endif // smtk_resource_Manager_h

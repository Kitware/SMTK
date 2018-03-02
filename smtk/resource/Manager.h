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

#include "smtk/common/UUID.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Event.h"
#include "smtk/resource/Metadata.h"
#include "smtk/resource/Name.h"
#include "smtk/resource/Resource.h"

#include <functional>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace smtk
{
namespace resource
{
class Manager;

/// A resource Manager is responsible for tracking currently allocaated
/// resources, creating new resources and serializing/deserializing resources
/// to/from disk. Resource types must first be registered with the Manager
/// before resources of they can be manipulated by the manager.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  /// Resource manager events result in function calls of this type.
  using Observer = std::function<void(Event, const Resource::Ptr&)>;

  smtkTypeMacroBase(Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Register a resource identified by its class type.
  template <typename ResourceType>
  bool registerResource(
    const std::function<ResourcePtr(const smtk::common::UUID&)>& create = nullptr);

  /// Construct a resource identified by its unique name.
  ResourcePtr create(const std::string&);

  /// Construct a resource identified by its type index.
  ResourcePtr create(const Resource::Index&);

  /// Construct a resource identified by its class type.
  template <typename ResourceType>
  smtk::shared_ptr<ResourceType> create();

  /// Construct a resource with a given UUID identified by its unique name.
  ResourcePtr create(const std::string&, const smtk::common::UUID&);

  /// Construct a resource with a given UUID identified by its type index.
  ResourcePtr create(const Resource::Index&, const smtk::common::UUID&);

  /// Construct a resource with a given UUID identified by its class type.
  template <typename ResourceType>
  smtk::shared_ptr<ResourceType> create(const smtk::common::UUID&);

  /// Returns the resource that relates to the given uuid.  If no association exists
  /// this will return a null pointer
  ResourcePtr get(const smtk::common::UUID& id);
  ConstResourcePtr get(const smtk::common::UUID& id) const;

  /// Returns the resource that relates to the given uuid.  If no association
  /// exists of this type, this will return a null pointer.
  template <typename ResourceType>
  smtk::shared_ptr<ResourceType> get(const smtk::common::UUID&);
  template <typename ResourceType>
  smtk::shared_ptr<const ResourceType> get(const smtk::common::UUID&) const;

  /// Returns the resource that relates to the given url.  If no association exists
  /// this will return a null pointer
  ResourcePtr get(const std::string&);
  ConstResourcePtr get(const std::string&) const;

  /// Returns the resource that relates to the given url.  If no association
  /// exists of this type, this will return a null pointer.
  template <typename ResourceType>
  smtk::shared_ptr<ResourceType> get(const std::string&);
  template <typename ResourceType>
  smtk::shared_ptr<const ResourceType> get(const std::string&) const;

  /// Returns a set of resources that have a given unique name <uniqueName>.
  std::set<ResourcePtr> find(const std::string&);

  /// Returns a set of resources that have a given type index..
  std::set<ResourcePtr> find(const Resource::Index&);

  /// Returns a set of resources that are of the type <ResourceType>.
  template <typename ResourceType>
  std::set<smtk::shared_ptr<ResourceType> > find();

  /// Add a resource identified by its type index. Returns true if the resource
  /// was added or already is part of this manager. If the resource is currently
  /// part of a different manager, we will reparent it to this manager.
  bool add(const Resource::Index&, const ResourcePtr&);

  /// Add a resource identified by its class type. Returns true if the resource
  /// was added or already is part of this manager.
  template <typename ResourceType>
  bool add(const smtk::shared_ptr<ResourceType>&);

  /// Returns true if the resource was added or already is part of this manager.
  bool add(const ResourcePtr&);

  /// Removes a resource from a given Manager. This doesn't explicitly release
  /// the memory of the resource, it only stops the tracking of the resource
  /// by the manager.
  bool remove(const ResourcePtr&);

  /// Visit each managed resource.
  void visit(const Resource::Visitor&) const;

  // We expose the underlying containers for both resources and metadata; this
  // means of access should not be necessary for most use cases.

  /// Return the set of resources.
  Container& resources() { return m_resources; }
  const Container& resources() const { return m_resources; }

  /// Return the map of metadata.
  MetadataContainer& metadata() { return m_metadata; }

  /**\brief Observe events related to this resource manager.
    *
    * Returns a handle that can be used to stop observations.
    *
    * If \a notifyOfCurrentState is true, then \a fn will be
    * immediately invoked with each registered resource type
    * and each registered resource.
    */
  int observe(const Observer& fn, bool notifyOfCurrentState = true);

  /// Stop observing events related to this resource manager.
  bool unobserve(int handle);

protected:
  /// Trigger an event (call all registered observers)
  void trigger(Event evt, const ResourcePtr& rsrc);

private:
  Manager();

  /// Register a resource identified by its type index.
  bool registerResource(Metadata&&);

  /// All resources are tracked using a map between the resource's UUID and a
  /// shared pointer to the resource itself.
  Container m_resources;

  std::map<int, Observer> m_observers;

  /// A container for all registered resource metadata.
  MetadataContainer m_metadata;
};

template <typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::create()
{
  return smtk::static_pointer_cast<ResourceType>(
    this->create(std::type_index(typeid(ResourceType)).hash_code()));
}

template <typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::create(const smtk::common::UUID& id)
{
  return smtk::static_pointer_cast<ResourceType>(
    this->create(std::type_index(typeid(ResourceType)).hash_code(), id));
}

template <typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::get(const smtk::common::UUID& id)
{
  return smtk::static_pointer_cast<ResourceType>(this->get(id));
}

template <typename ResourceType>
smtk::shared_ptr<const ResourceType> Manager::get(const smtk::common::UUID& id) const
{
  return smtk::static_pointer_cast<const ResourceType>(this->get(id));
}

template <typename ResourceType>
smtk::shared_ptr<ResourceType> Manager::get(const std::string& url)
{
  return smtk::static_pointer_cast<ResourceType>(this->get(url));
}

template <typename ResourceType>
smtk::shared_ptr<const ResourceType> Manager::get(const std::string& url) const
{
  return smtk::static_pointer_cast<const ResourceType>(this->get(url));
}

template <typename ResourceType>
std::set<smtk::shared_ptr<ResourceType> > Manager::find()
{
  Resource::Index index(typeid(ResourceType).hash_code());
  std::set<Resource::Index> validIndices;
  for (auto& metadatum : m_metadata)
  {
    if (metadatum.m_parentIndices.find(index) != metadatum.m_parentIndices.end())
    {
      validIndices.insert(metadatum.index());
    }
  }

  std::set<smtk::shared_ptr<ResourceType> > values;

  typedef Container::index<IndexTag>::type ResourcesByIndex;
  ResourcesByIndex& resources = this->m_resources.get<IndexTag>();
  for (auto& idx : validIndices)
  {
    auto resourceItRange = resources.equal_range(idx);
    for (auto& it = resourceItRange.first; it != resourceItRange.second; ++it)
    {
      values.insert(smtk::static_pointer_cast<ResourceType>(*it));
    }
  }

  return values;
}

template <typename ResourceType>
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
template <typename T>
class is_derived_resource
{
  class No
  {
  };
  class Yes
  {
    No no[2];
  };

  template <typename C>
  static Yes Test(typename C::ParentResource*);
  template <typename C>
  static No Test(...);

public:
  enum
  {
    value = sizeof(Test<T>(nullptr)) == sizeof(Yes)
  };
};

// The signature for our index generator has two template parameters.
template <typename ResourceType, bool derived_resource>
struct resource_index_set_generator;

// This partial template specialization deals with the case where
// <ResourceType> is not derived from another Resource. In this case, only the
// index for the Resource is added to the set of associated indices.
template <typename ResourceType>
struct resource_index_set_generator<ResourceType, false>
{
  static std::set<Resource::Index> indices()
  {
    std::set<Resource::Index> indices;
    indices.insert(typeid(ResourceType).hash_code());

    return indices;
  }
};

// This partial template specialization deals with the case where
// <ResourceType> is derived from another Resource. In this case, we
// recursively add the parent Resource's associated indices to the returned
// set.
template <typename ResourceType>
struct resource_index_set_generator<ResourceType, true>
{
  static std::set<Resource::Index> indices()
  {
    std::set<Resource::Index> indices;
    indices.insert(std::type_index(typeid(ResourceType)).hash_code());

    std::set<Resource::Index> parentIndices =
      resource_index_set_generator<typename ResourceType::ParentResource,
        is_derived_resource<typename ResourceType::ParentResource>::value>::indices();
    indices.insert(parentIndices.begin(), parentIndices.end());

    return indices;
  }
};
}

template <typename ResourceType>
bool Manager::registerResource(const std::function<ResourcePtr(const smtk::common::UUID&)>& create)
{
  // For standard Resources, the metadata is comprised of the following:
  // Unique Name: either the "type_name" field or (if the former does not exist)
  //              the uniqueName() value
  // Index: the hash of the type_index
  // Parent Indices: a set of indices constructed by traversing parent resources
  //                 (defined using the typedef "ParentResource")
  // Create Functor: either a user-defined functor or the default
  //                 ResourceType::create method

  return Manager::registerResource(Metadata(
    smtk::resource::name<ResourceType>(), std::type_index(typeid(ResourceType)).hash_code(),
    (detail::resource_index_set_generator<ResourceType,
      detail::is_derived_resource<ResourceType>::value>::indices()),
    (create ? create : [](const smtk::common::UUID& id) {
      Resource::Ptr resource = ResourceType::create();
      resource->setId(id);
      return resource;
    })));
}
}
}

#endif // smtk_resource_Manager_h

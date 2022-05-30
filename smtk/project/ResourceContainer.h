//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_ResourceContainer_h
#define smtk_project_ResourceContainer_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Deprecation.h"
#include "smtk/common/TypeName.h"

#include "smtk/project/Tags.h"

#include "smtk/resource/Container.h"

namespace smtk
{
namespace project
{
namespace detail
{
SMTKCORE_EXPORT const std::string& role(const smtk::resource::ResourcePtr& r);
}

class Project;

/// A ResourceContainer is a container for a Project's Resources. It holds a
/// searchable collection of its Resources.
class SMTKCORE_EXPORT ResourceContainer
{
  friend class Project;

public:
  /// A multi-index container for accessing resources. This class is primarily
  /// intended to be used in the implementation of smtk::resource::Manager only.
  typedef boost::multi_index_container<
    smtk::resource::ResourcePtr,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_unique<
        boost::multi_index::tag<IdTag>,
        boost::multi_index::global_fun<
          const smtk::resource::ResourcePtr&,
          const smtk::common::UUID&,
          &smtk::resource::detail::id>>,
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<IndexTag>,
        boost::multi_index::global_fun<
          const smtk::resource::ResourcePtr&,
          smtk::resource::Resource::Index,
          &smtk::resource::detail::index>>,
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<NameTag>,
        boost::multi_index::global_fun<
          const smtk::resource::ResourcePtr&,
          std::string,
          &smtk::resource::detail::name>>,
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<LocationTag>,
        boost::multi_index::global_fun<
          const smtk::resource::ResourcePtr&,
          const std::string&,
          &smtk::resource::detail::location>>,
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<RoleTag>,
        boost::multi_index::global_fun<
          const smtk::resource::ResourcePtr&,
          const std::string&,
          &smtk::project::detail::role>>>>
    Container;

  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;

  /// A property key for accessing string-valued roles assigned to a resource
  /// held by a project.
  static constexpr const char* const role_name = "project_role";

  ~ResourceContainer();

  /// Register a resource type according to its typename, type index or class
  /// type.
  bool registerResource(const std::string&);
  bool registerResource(const smtk::resource::Resource::Index&);
  template<typename ResourceType>
  bool registerResource();

  /// Register a set of resource types according to their typenames.
  bool registerResources(const std::set<std::string>&);

  /// Unregister a resource type according to its typename, type index or class
  /// type.
  bool unregisterResource(const std::string&);
  bool unregisterResource(const smtk::resource::Resource::Index&);
  template<typename ResourceType>
  bool unregisterResource();

  /// Returns the resource that relates to the given uuid.  If no association
  /// exists this will return a null pointer.
  smtk::resource::ResourcePtr get(const smtk::common::UUID& id);
  smtk::resource::ConstResourcePtr get(const smtk::common::UUID& id) const;
  template<typename ResourceType>
  smtk::shared_ptr<ResourceType> get(const smtk::common::UUID&);
  template<typename ResourceType>
  smtk::shared_ptr<const ResourceType> get(const smtk::common::UUID&) const;

  /// Returns the resource that relates to the given url.  If no association
  /// exists this will return a null pointer
  smtk::resource::ResourcePtr get(const std::string&);
  smtk::resource::ConstResourcePtr get(const std::string&) const;
  template<typename ResourceType>
  smtk::shared_ptr<ResourceType> get(const std::string&);
  template<typename ResourceType>
  smtk::shared_ptr<const ResourceType> get(const std::string&) const;

  ///@{
  /// Returns the set of resources that relates to the given role.  If no association
  /// exists this will return an empty set.
  std::set<smtk::resource::ResourcePtr> findByRole(const std::string&);
  std::set<smtk::resource::ConstResourcePtr> findByRole(const std::string&) const;
  template<typename ResourceType>
  std::set<smtk::shared_ptr<ResourceType>> findByRole(const std::string&);
  template<typename ResourceType>
  std::set<smtk::shared_ptr<const ResourceType>> findByRole(const std::string&) const;
  ///@}

  /// Returns a set of resources that have a given typename, type index or class
  /// type.
  std::set<smtk::resource::ResourcePtr> find(const std::string&) const;
  std::set<smtk::resource::ResourcePtr> find(const smtk::resource::Resource::Index&) const;
  template<typename ResourceType>
  std::set<smtk::shared_ptr<ResourceType>> find() const;

  /// Add a resource identified by its index or class type and by its role.
  /// Returns true if the resource was added or already is part of this manager.
  bool add(const smtk::resource::ResourcePtr&, const std::string& role = std::string());
  bool add(
    const smtk::resource::Resource::Index&,
    const smtk::resource::ResourcePtr&,
    std::string role = std::string());
  template<typename ResourceType>
  bool add(const smtk::shared_ptr<ResourceType>&, const std::string& role = std::string());

  /// Removes a resource from a given Project. This doesn't explicitly release
  /// the memory of the resource, it only stops the tracking of the resource
  /// by the Project.
  bool remove(const smtk::resource::ResourcePtr&);

  /// Return a whitelist of typenames of allowed resources. If Empty, all
  /// Resource types are allowed.
  const std::set<std::string>& types() const { return m_types; }
  std::set<std::string>& types() { return m_types; }

  std::shared_ptr<smtk::resource::Manager> manager() const { return m_manager.lock(); }
  void setManager(const std::weak_ptr<smtk::resource::Manager>& manager) { m_manager = manager; }

  const_iterator begin() const { return m_resources.begin(); }
  iterator begin() { return m_resources.begin(); }

  const_iterator end() const { return m_resources.end(); }
  iterator end() { return m_resources.end(); }

  bool empty() const { return m_resources.empty(); }
  std::size_t size() const { return m_resources.size(); }
  void clear() { m_resources.clear(); }

private:
  ResourceContainer(const smtk::project::Project*, const std::weak_ptr<smtk::resource::Manager>&);

  const smtk::project::Project* m_project;
  std::weak_ptr<smtk::resource::Manager> m_manager;
  std::set<std::string> m_types;
  Container m_resources;
  int m_undefinedRoleCounter{ 0 };
};

template<typename ResourceType>
bool ResourceContainer::registerResource()
{
  return this->registerResource(smtk::common::typeName<ResourceType>());
}

template<typename ResourceType>
bool ResourceContainer::unregisterResource()
{
  return this->unregisterResource(smtk::common::typeName<ResourceType>());
}

template<typename ResourceType>
smtk::shared_ptr<ResourceType> ResourceContainer::get(const smtk::common::UUID& id)
{
  return smtk::static_pointer_cast<ResourceType>(this->get(id));
}

template<typename ResourceType>
smtk::shared_ptr<const ResourceType> ResourceContainer::get(const smtk::common::UUID& id) const
{
  return smtk::static_pointer_cast<const ResourceType>(this->get(id));
}

template<typename ResourceType>
smtk::shared_ptr<ResourceType> ResourceContainer::get(const std::string& url)
{
  return smtk::static_pointer_cast<ResourceType>(this->get(url));
}

template<typename ResourceType>
smtk::shared_ptr<const ResourceType> ResourceContainer::get(const std::string& url) const
{
  return smtk::static_pointer_cast<const ResourceType>(this->get(url));
}

template<typename ResourceType>
std::set<smtk::shared_ptr<ResourceType>> ResourceContainer::findByRole(const std::string& role)
{
  std::set<smtk::shared_ptr<ResourceType>> cast_set;
  for (const auto& resourceptr : this->findByRole(role))
  {
    auto castedResource = smtk::dynamic_pointer_cast<ResourceType>(resourceptr);
    if (castedResource != nullptr)
    {
      cast_set.insert(castedResource);
    }
  }
  return cast_set;
}

template<typename ResourceType>
std::set<smtk::shared_ptr<const ResourceType>> ResourceContainer::findByRole(
  const std::string& role) const
{
  std::set<smtk::shared_ptr<ResourceType>> cast_set;
  for (auto resourceptr : this->findByRole(role))
  {
    auto castedResource = smtk::dynamic_pointer_cast<const ResourceType>(resourceptr);
    if (castedResource != nullptr)
    {
      cast_set.insert(castedResource);
    }
  }
  return cast_set;
}

template<typename ResourceType>
std::set<smtk::shared_ptr<ResourceType>> ResourceContainer::find() const
{
  std::set<smtk::shared_ptr<ResourceType>> cast_set;
  for (const auto& resourceptr : this->find(typeid(ResourceType).hash_code()))
  {
    cast_set.insert(std::dynamic_pointer_cast<ResourceType>(resourceptr));
  }
  return cast_set;
}

template<typename ResourceType>
bool ResourceContainer::add(const smtk::shared_ptr<ResourceType>& resource, const std::string& role)
{
  return this->add(std::type_index(typeid(ResourceType)).hash_code(), resource, role);
}
} // namespace project
} // namespace smtk

#endif

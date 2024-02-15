//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Resource_h
#define smtk_resource_Resource_h

#include "smtk/CoreExports.h"

#include "smtk/common/UUID.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Lock.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/ResourceLinks.h"

#include "smtk/resource/query/BadTypeError.h"
#include "smtk/resource/query/Queries.h"

namespace units
{
struct System;
}

#include <string>
#include <typeindex>
#include <unordered_map>

namespace smtk
{
namespace operation
{
class Operation;
}
namespace resource
{
class CopyOptions;

/// Operations need the ability to lock and unlock resources, but no additional
/// access privilege is required. We therefore use the PassKey pattern to grant
/// Operation access to a resource's lock.
/// Outside of operations, if you need to acquire a lock you must use a
/// smtk::resource::ScopedLockSetGuard to ensure that locks are freed. Do
/// not ever use the "new" operator to construct a ScopedLockSetGuard, as
/// leaking it could cause deadlocks.
class Key
{
  friend class operation::Operation;
  friend class smtk::resource::ScopedLockSetGuard;
  Key() = default;
};

template<typename Self, typename Parent>
class DerivedFrom;

class Manager;
class Metadata;

/**\brief An abstract base class for SMTK resources.
  *
  * Resources represent a collection of persistent objects written
  * to a single file (i.e., a document). While it is possible –
  * via resource::Links – for a document to reference objects external
  * to a file, resources are intended mostly to be self-contained.
  */
class SMTKCORE_EXPORT Resource : public PersistentObject
{
public:
  typedef std::size_t Index;
  typedef smtk::resource::Metadata Metadata;
  typedef detail::ResourceLinks Links;
  typedef detail::ResourceProperties Properties;
  typedef query::Queries Queries;

  friend class Manager;

  template<typename Child, typename Parent>
  friend class DerivedFrom;

  static const Resource::Index type_index;

  /// A role for components that implies their visibility in renderings
  /// should be linked to another object.
  ///
  /// When an object, A, without renderable geometry is linked to object(s), B,
  /// with renderable geometry, SMTK user-interface elements should show
  /// visibility-control badges for both A and B.
  static constexpr smtk::resource::Links::RoleType VisuallyLinkedRole = -4;

  smtkTypeMacro(smtk::resource::Resource);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);
  ~Resource() override;

  /// index is a compile-time intrinsic of the derived resource; as such, it
  /// cannot be set.
  virtual Index index() const { return std::type_index(typeid(*this)).hash_code(); }

  /// given a resource type, return whether or not this resource is or is
  /// derived from the resource described by the index.
  template<class ResourceType>
  bool isOfType() const
  {
    return this->isOfType(std::type_index(typeid(ResourceType)).hash_code());
  }

  /// given a resource index, return whether or not this resource is or is
  /// derived from the resource described by the index.
  virtual bool isOfType(const Index& index) const;

  /// given a resource's unique name, return whether or not this resource is or
  /// is derived from the resource described by the name.
  virtual bool isOfType(const std::string& typeName) const;

  /// given a resource's unique name, return the number of generations derived
  /// from the resource described from the name (or a negative value if this
  /// resource is not derived from the input resource type).
  virtual int numberOfGenerationsFromBase(const std::string& typeName) const;

  /// id and location are run-time intrinsics of the derived resource; we need
  /// to allow the user to reset these values.
  const smtk::common::UUID& id() const override { return m_id; }
  const std::string& location() const { return m_location; }

  bool setId(const smtk::common::UUID& myID) override;
  virtual bool setLocation(const std::string& location);

  /// Return the user-assigned name of the resource.
  ///
  /// If no name has been assigned, return the stem of its filename.
  /// You may use isNameSet() to determine whether the returned name
  /// is generated or assigned.
  std::string name() const override;
  virtual bool setName(const std::string& name);
  bool isNameSet() { return !m_name.empty(); }

  /// Indicate whether the resource is in sync with its location.
  ///
  /// Resources that have a non-empty location and are identical to
  /// the data stored at location are clean. All other resources are dirty.
  ///
  /// Operations that are write-operators (i.e., not read-only) should mark
  /// resources as modified. Saving a resource using its metadata's write
  /// method will mark the resource as clean. Loading a resource using
  /// its metadata's read method should return a clean resource.
  virtual bool clean() const { return m_clean; }
  void setClean(bool state = true);

  /// Mark the resource to indicate it is about to removed (meaning it is being removed from memory
  /// not necessarily for deletion)
  void setMarkedForRemoval(bool val) { m_markedForRemoval = val; }

  /// Return whether the object is marked for removal
  virtual bool isMarkedForRemoval() const { return m_markedForRemoval; }
  /// Resources that are managed have a non-null pointer to their manager.
  ManagerPtr manager() const { return m_manager.lock(); }

  /// Given a resource component's UUID, return the resource component.
  virtual ComponentPtr find(const smtk::common::UUID& compId) const = 0;

  /// Given a component's UUID, return a raw pointer to the component.
  virtual Component* component(const smtk::common::UUID& compId) const;

  /**\brief A templated version of `component()` that casts its result to a type.
    *
    * This method performs a dynamic cast, so it may return nullptr even
    * if there is a component with a matching \a uuid (in the case that it
    * is of a different type than \a ComponentType).
    */
  template<typename ComponentType>
  ComponentType* componentAs(const smtk::common::UUID& uuid) const
  {
    return dynamic_cast<ComponentType*>(this->component(uuid));
  }

  /// given a std::string describing a query, return a functor for performing
  /// the query (accepts component as input, returns true if the component
  /// satisfies the query parameters).
  virtual std::function<bool(const Component&)> queryOperation(const std::string&) const;

  /// visit all components in a resource.
  virtual void visit(std::function<void(const ComponentPtr&)>& v) const = 0;

  ComponentSet filter(const std::string& queryString) const;

  /// given a a std::string describing a query and a type of container, return a
  /// set of components that satisfy both.  Note that since this uses a dynamic
  /// pointer cast this can be slower than other find methods.
  template<typename Collection>
  Collection filterAs(const std::string& queryString) const;

  Links& links() override { return m_links; }
  const Links& links() const override { return m_links; }

  Properties& properties() override { return m_properties; }
  const Properties& properties() const override { return m_properties; }

  const Queries& queries() const { return m_queries; }
  Queries& queries() { return m_queries; }

  /// classes that are granted permission to the key may retrieve the resource's
  /// lock.
  Lock& lock(Key()) const { return m_lock; }

  /// Anyone can query whether or not the resource is locked.
  LockType locked() const { return m_lock.state(); }

  Resource(Resource&&) noexcept;

  ///@{
  /// \brief Sets and Gets the system of units used by this resource
  virtual bool setUnitsSystem(const shared_ptr<units::System>& unitsSystem);
  const shared_ptr<units::System>& unitsSystem() const { return m_unitsSystem; }
  ///@}

  /// Set/get the "type" of a resource's template.
  ///
  /// A resource template-type is not required, but if present it can be used to
  /// register updaters for migrating from an old template to a newer version.
  ///
  /// The default implementation returns an invalid string token (indicating
  /// the resource does not support templates). Subclasses must override this
  /// method if they wish to support document templates.
  virtual bool setTemplateType(const smtk::string::Token& templateType);
  virtual const smtk::string::Token& templateType() const;

  /// Set/get the version number of the template this instance of the resource is based upon.
  ///
  /// If non-zero, this number indicates the version number of the
  /// template (i.e., the attribute/item definitions for attribute resources)
  /// the current resource draws from. It is used during the update process to determine
  /// which updaters are applicable.
  ///
  /// The default implementation always returns 0, indicating version numbers
  /// are not supported by resources of this type. Subclasses must override
  /// these methods if they wish to support document-template versioning.
  virtual bool setTemplateVersion(std::size_t templateVersion);
  virtual std::size_t templateVersion() const;

  /// Create an empty, un-managed clone of this resource instance.
  ///
  /// Note that it is valid to (and the default implementation does) return
  /// a null pointer to indicate a resource cannot be cloned.
  ///
  /// This method may be used create a resource for either copying (i.e.,
  /// in order to create a new resource with the same structure as the
  /// original but with distinct UUIDs) or updating (i.e., in order to
  /// transform a resource from one revision of a template to another
  /// while preserving UUIDs).
  ///
  /// This method generally does not copy the content of a resource.
  /// Use the copy() method on the returned clone if you wish to copy
  /// resource-specific persistent objects.
  /// However, the \a options **will** determine whether ancillary data
  /// such as template-specific data, the unit system, and other information
  /// not related to the information being modeled by the resource is
  /// present in the returned clone.
  virtual std::shared_ptr<Resource> emptyClone(CopyOptions& options);

  /// Copy data from a \a source resource into this resource.
  ///
  /// This method must be subclassed by resources that wish to support copying;
  /// the default implementation simply returns false.
  ///
  /// Call this method on the result of emptyClone() to copy persistent
  /// objects, properties, links, and other data from the \a source into
  /// this resource.
  /// Note that link-data is copied wholesale but not adjusted to reference
  /// copied objects rather than objects in the \a source resource;
  /// use resolveCopy() to perform this adjustment.
  ///
  /// If this method returns true, you should call resolveCopy() as well.
  /// The resolveCopy() method resolves external references using data stored
  /// in \a options by the copy() method. The two methods (copy() and resolveCopy())
  /// allow duplication of _multiple resources_ at once with references among them
  /// properly translated. This is accomplished by calling copy() on each resource
  /// to be processed and then calling resolveCopy() on each resource.
  ///
  /// This method will always produce components that mirror the \a source components
  /// but have distinct UUIDs. On completion, the \a options object holds a map relating
  /// the \a source components to their copies in this resource.
  virtual bool copy(const std::shared_ptr<const Resource>& source, CopyOptions& options);

  /// Resolve internal and external references among components copied from a \a source resource.
  ///
  /// After components are copied from the \a source, there may still be references to the
  /// original components; this method resolves those references to the copied components.
  virtual bool resolveCopy(const std::shared_ptr<const Resource>& source, CopyOptions& options);

protected:
  // Derived resources should inherit
  // smtk::resource::DerivedFrom<Self, smtk::resource::Resource>. Resource's
  // constructors are declared private to enforce this relationship.
  Resource(const smtk::common::UUID&, ManagerPtr manager = nullptr);
  Resource(ManagerPtr manager = nullptr);

  WeakManagerPtr m_manager;
  std::shared_ptr<units::System> m_unitsSystem;

private:
  /// Instances of this internal class are passed to resource::Manager to
  /// modify a resource's UUID in-place while preserving the manager's indexing.
  ///
  /// This class should only be used by Resource::setId().
  struct SetId
  {
    SetId(const smtk::common::UUID& uid)
      : m_id(uid)
    {
    }

    void operator()(ResourcePtr& resource) const { resource->m_id = m_id; }

    smtk::common::UUID id() const { return m_id; }

    const smtk::common::UUID& m_id;
  };

  /// Instances of this internal class are passed to resource::Manager to modify
  /// a resource's location in-place while preserving the manager's indexing.
  ///
  /// This class should only be used by Resource::setLocation().
  struct SetLocation
  {
    SetLocation(const std::string& url)
      : m_url(url)
    {
    }

    void operator()(ResourcePtr& resource) const { resource->m_location = m_url; }

    std::string location() const { return m_url; }

    const std::string& m_url;
  };

  smtk::common::UUID m_id;
  std::string m_location;
  std::string m_name;
  /// True when m_location is in sync with this instance.
  bool m_clean;

  Links m_links;
  Properties m_properties;
  Queries m_queries;
  bool m_markedForRemoval = false;
  mutable Lock m_lock;
};

template<typename Collection>
Collection Resource::filterAs(const std::string& queryString) const
{
  // Construct a query operation from the query string
  auto queryOp = this->queryOperation(queryString);

  // Construct a component set to fill
  Collection col;

  // Visit each component and add it to the set if it satisfies the query
  smtk::resource::Component::Visitor visitor = [&](const ComponentPtr& component) {
    if (queryOp(*component))
    {
      auto entry =
        std::dynamic_pointer_cast<typename Collection::value_type::element_type>(component);
      if (entry)
      {
        col.insert(col.end(), entry);
      }
    }
  };

  this->visit(visitor);

  return col;
}

/// Given an object, return a query for interrogating it.
///
/// This method will test whether the object is a resource
/// (in which case it asks the resource to construct the query)
/// or a component (in which case it asks the component's
/// owning resource to construct the query).
template<typename QueryType>
SMTKCORE_NO_EXPORT QueryType& queryForObject(const PersistentObject& object)
{
  const auto* resource = dynamic_cast<const Resource*>(&object);
  if (!resource)
  {
    const auto* const component = dynamic_cast<const Component*>(&object);
    if (component)
    {
      resource = component->resource().get();
    }
  }
  if (resource)
  {
    return resource->queries().get<QueryType>();
  }
  throw query::BadTypeError(smtk::common::typeName<QueryType>());
}
} // namespace resource
} // namespace smtk

#endif // smtk_resource_Resource_h

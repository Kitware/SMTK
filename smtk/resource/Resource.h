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
  /// visibility-control badges for both A and B. This is indicated by a
  /// link from A to B.
  static constexpr smtk::resource::Links::RoleType VisuallyLinkedRole = -4;

  smtkTypeMacro(smtk::resource::Resource);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);
  ~Resource() override;

  ///@name Resource Type Introspection
  ///@{
  /// Resources provide an integer type-id common to their class definition
  /// as well as methods to detect when resources are derived from a common
  /// ancestor class.

  /// Index is a compile-time intrinsic of the derived resource's type; as such, it
  /// cannot be set.
  virtual Index index() const { return std::type_index(typeid(*this)).hash_code(); }

  /// Given a resource type, return whether or not this resource is or is
  /// derived from the resource described by the index.
  template<class ResourceType>
  bool isOfType() const
  {
    return this->isOfType(std::type_index(typeid(ResourceType)).hash_code());
  }

  /// Given a resource index, return whether or not this resource is or is
  /// derived from the resource described by the index.
  virtual bool isOfType(const Index& index) const;

  /// Given a resource's unique name, return whether or not this resource is or
  /// is derived from the resource described by the name.
  virtual bool isOfType(const std::string& typeName) const;

  /// Given a resource's unique name, return the number of generations derived
  /// from the resource described from the name (or a negative value if this
  /// resource is not derived from the input resource type).
  virtual int numberOfGenerationsFromBase(const std::string& typeName) const;
  ///@}

  ///@name Unique Resource IDs
  ///@{
  /// Because resources inherit PersistentObject, they (and the components
  /// they own) must provide unique identifiers.

  /// Set/get the UUID of a resource.
  ///
  /// This may be modified at construction but should not change afterward
  /// unless there are exceptional circumstances.
  const smtk::common::UUID& id() const override { return m_id; }
  bool setId(const smtk::common::UUID& myID) override;
  ///@}

  ///@name Resource Location
  ///@{
  /// Resources must have a URL where they reside when not in memory.

  /// Set/get the location (a URL) where the resource is stored persistently.
  ///
  /// This may change when a user chooses to "Save As…" a different filename.
  const std::string& location() const { return m_location; }
  virtual bool setLocation(const std::string& location);
  ///@}

  ///@name Naming
  ///@{
  /// As they inherit PersistentObject, resources must provide a name.
  /// By default, if a resource has no name but has a location (URL), the
  /// stem of its filename is used as its name.

  /// Set/get the user-assigned name of the resource.
  ///
  /// If no name has been assigned, return the stem of its filename.
  /// You may use isNameSet() to determine whether the returned name
  /// is generated or assigned.
  std::string name() const override;
  virtual bool setName(const std::string& name);
  bool isNameSet() const { return !m_name.empty(); }
  ///@}

  ///@name Clean/Dirty State
  ///@{
  /// Indicate whether the resource is in sync with the persistent storage at its location().

  /// Resources that have a non-empty location and are identical to
  /// the data stored at location are clean. All other resources are dirty.
  ///
  /// Operations that are write-operators (i.e., not read-only) should mark
  /// resources as modified. Saving a resource using its metadata's write
  /// method will mark the resource as clean. Loading a resource using
  /// its metadata's read method should return a clean resource.
  virtual bool clean() const { return m_clean; }
  void setClean(bool state = true);
  ///@}

  ///@name Removing (un-managing) Resources
  ///@{
  /// Resources are typically owned by an smtk::resource::Manager.
  /// However, operations may sometimes request that a resource be
  /// unmanaged (released from the manager and thus removed from memory).
  /// These methods provide information about the ownership of this resource.

  /// Mark the resource to indicate it is about to removed (meaning it is being removed from memory
  /// not necessarily for deletion)
  void setMarkedForRemoval(bool val) { m_markedForRemoval = val; }

  /// Return whether the object is marked for removal
  virtual bool isMarkedForRemoval() const { return m_markedForRemoval; }

  /// Resources that are managed have a non-null pointer to their manager.
  ManagerPtr manager() const { return m_manager.lock(); }
  ///@}

  ///@name Finding, Visiting, and Filtering Components.
  ///@{
  /// A resource owns a collection of components.
  /// These methods provide a consistent API for discovering, filtering,
  /// and iterating a resource's components.

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
  ///@}

  ///@name Cross-resource Links
  ///@{
  /// Links provide a way to express relationships between components
  /// in other resources (as well as within the same resource).
  /// These methods provide access to the storage for link data.

  /// Fetch the links stored for this resource and its components.
  Links& links() override { return m_links; }
  const Links& links() const override { return m_links; }
  ///@}

  ///@name Property Data
  ///@{
  /// Resources and their components may be annotated with free-form properties.
  /// These methods provide read/write and read-only access to the annotations.
  Properties& properties() override { return m_properties; }
  const Properties& properties() const override { return m_properties; }
  ///@}

  ///@name Resource Queries
  ///@{
  /// Sometimes different types of resources (i.e., instances of different subclasses)
  /// may need a uniform API to provide information; query objects exist for this purpose
  /// and these methods provide read/write and read-only access to a query factory and
  /// cached data related to these queries.
  const Queries& queries() const { return m_queries; }
  Queries& queries() { return m_queries; }
  ///@}

  ///@name Resource Locking
  ///@{
  /// SMTK assumes that only one thread at a time may modify a resource and its
  /// components.
  /// Resources may be locked using the methods provided by the resource.
  /// Components may not be locked, so threads block access to the entire resource.
  ///
  /// Operations are designed to acquire resource locks for you before the operation
  /// is run. You should strive to use the operation framework for this rather than
  /// to manually lock resources.

  /// Classes that are granted permission to the key may retrieve the resource's
  /// lock. This prevents other threads from modifying the resource simultaneously.
  Lock& lock(Key()) const { return m_lock; }

  /// Anyone can query whether or not the resource is locked.
  LockType locked() const { return m_lock.state(); }
  ///@}

  Resource(Resource&&) noexcept;

  ///@name Units and Dimensional Analysis.
  ///@{
  /// Resources may own a "unit system" (i.e., a set of transformations that
  /// can be applied to convert dimensional measurements between scales in
  /// a consistent way) in order to accept user inputs in convenient units
  /// while ensuring SMTK's outputs are dimensionally consistent.
  ///
  /// Many resources may share reference to a common unit system, but this
  /// is not enforced; it is also possible for resources to have distinct
  /// unit systems.

  /// \brief Sets the system of units used by this resource.
  virtual bool setUnitsSystem(const shared_ptr<units::System>& unitsSystem);
  /// \brief Gets the system of units used by this resource.
  const shared_ptr<units::System>& unitsSystem() const { return m_unitsSystem; }
  ///@}

  ///@name Resource Templates
  ///@{
  /// A resource _may_ have a template (also called a "schema," "theme," or
  /// "convention") describing a convention for how its contents are structured.
  /// Subclasses of the base resource class may override these methods to
  /// advertise their ability to model their structure according to multiple
  /// templates (by providing a valid template name and version).

  /// Set/get the "type" of a resource's template.
  ///
  /// A resource template-type is not required, but if present it can be used to
  /// register updaters for migrating from an old template to a newer version.
  ///
  /// The default implementation returns an invalid string token (indicating
  /// the resource does not support templates). Subclasses must override this
  /// method if they wish to support document templates.
  virtual bool setTemplateType(const smtk::string::Token& templateType);
  virtual smtk::string::Token templateType() const;

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
  ///@}

  ///@name Copying and Updating
  ///@{
  /// One resource may be used as a template for other resources. (This is also
  /// known as "prototypal inheritance.") Subclasses of the base resource class
  /// which implement these methods provide a way to clone, copy, and/or update
  /// resource templates/schemas.

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
  /// This method does not copy the user-authored content of a resource.
  /// Use the copyData() method on the returned clone if you wish to copy
  /// that data.
  ///
  /// However, the \a options **will** determine whether ancillary data
  /// such as template-specific data, the unit system, and other information
  /// not related to the information being modeled by the resource is
  /// present in the returned clone.
  virtual std::shared_ptr<Resource> clone(CopyOptions& options) const;

  /// Copy data from a \a source resource into this resource.
  ///
  /// This method must be subclassed by resources that wish to support copying;
  /// the default implementation simply returns false.
  ///
  /// Call this method on the result of clone() to copy persistent objects,
  /// properties, and other self-contained resource-specific data from the \a source
  /// into this resource.
  ///
  /// If this method returns true, you should call copyRelations() as well.
  /// The copyRelations() method adds any requested references between objects (both
  /// in the same and in external resources) using data stored in \a options by the
  /// copyData() method. The two methods (copyData() and copyRelations())
  /// allow duplication of _multiple resources_ at once with references among them
  /// properly translated. This is accomplished by calling copyData() on each resource
  /// to be processed and _then_ calling copyRelations() on each resource.
  ///
  /// This method will always produce components that mirror the \a source components
  /// but have distinct UUIDs. On completion, the \a options object holds a map relating
  /// the \a source components to their copies in this resource.
  virtual bool copyData(const std::shared_ptr<const Resource>& source, CopyOptions& options);

  /// Resolve internal and external references among components copied from a \a source resource.
  ///
  /// After components are copied from the \a source, there may still be references to the
  /// original components; this method resolves those references to the copied components.
  virtual bool copyRelations(const std::shared_ptr<const Resource>& source, CopyOptions& options);

  /// Copy the units system from \a rsrc into this resource as specified by \a options.
  ///
  /// This method is provided so subclasses that implement clone() do
  /// not need to repeat code common to all resources.
  void copyUnitSystem(const std::shared_ptr<const Resource>& rsrc, const CopyOptions& options);

  /// Copy all property data from \a rsrc, mapping them along the way via \a options.
  ///
  /// This method is intended for use by subclasses of Resource from within their
  /// copyData() implementation.
  ///
  /// Note that this method must be called **after** components have been copied from
  /// \a rsrc as it relies upon \a options.objectMapping() to contain entries that
  /// map original component UUIDs to their new UUIDs.
  ///
  /// If \a options.copyComponents() is false, any properties on the resource
  /// and non-component properties (property entries with a UUID that do not
  /// correspond to a component in \a rsrc) will still be copied.
  void copyProperties(const std::shared_ptr<const Resource>& rsrc, const CopyOptions& options);
  ///@}

protected:
  // Derived resources should inherit
  // smtk::resource::DerivedFrom<Self, smtk::resource::Resource>. Resource's
  // constructors are declared private to enforce this relationship.
  Resource(const smtk::common::UUID&, ManagerPtr manager = nullptr);
  Resource(ManagerPtr manager = nullptr);

  /// Copy **all** property data for \a sourceId from \a rsrc into \a targetId of _this_ resource.
  ///
  /// This is called from copyProperties() for each entry in \a rsrc, possibly
  /// with the \a options objectMappings() applied.
  ///
  /// This method returns true if any properties were copied and false otherwise.
  ///
  /// Note that some properties may not be copy-constructible – these properties
  /// will not be copied and, depending on \a options, may result in a log message.
  bool copyPropertiesForId(
    const std::shared_ptr<Resource>& rsrc,
    const smtk::common::UUID& sourceId,
    const smtk::common::UUID& targetId,
    const CopyOptions& options);

  /// Copy links from \a rsrc (except those excluded by \a options), mapping them along the way.
  ///
  /// The objectMapping() in \a options is used to transform links, so that any UUID present in
  /// the mapping keys is transformed to the UUID of the map's corresponding value.
  ///
  /// This method is intended for use by subclasses of Resource from within their
  /// copyRelations() implementation.
  void copyLinks(const std::shared_ptr<Resource>& rsrc, const CopyOptions& options);

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

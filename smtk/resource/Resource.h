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

#include "smtk/resource/Lock.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/ResourceLinks.h"

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
/// Operations need the ability to lock and unlock resources, but no additional
/// access privilege is required. We therefore use the PassKey pattern to grant
/// Operation access to a resource's lock.
class Key
{
  friend class operation::Operation;
  Key() {}
};

template <typename Self, typename Parent>
class DerivedFrom;

class Manager;
class Metadata;

/// An abstract base class for SMTK resources.
class SMTKCORE_EXPORT Resource : public PersistentObject
{
public:
  typedef std::size_t Index;
  typedef smtk::resource::Metadata Metadata;
  typedef std::function<void(const ResourcePtr&)> Visitor;
  typedef ResourceLinks Links;

  friend class Manager;

  template <typename Child, typename Parent>
  friend class DerivedFrom;

  smtkTypedefs(smtk::resource::Resource);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);
  virtual ~Resource();

  /// index is a compile-time intrinsic of the derived resource; as such, it
  /// cannot be set.
  virtual Index index() const { return std::type_index(typeid(*this)).hash_code(); }

  /// given a resource type, return whether or not this resource is or is
  /// derived from the resource described by the index.
  template <class ResourceType>
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

  /// id and location are run-time intrinsics of the derived resource; we need
  /// to allow the user to reset these values.
  const smtk::common::UUID& id() const override { return m_id; }
  const std::string& location() const { return m_location; }

  bool setId(const smtk::common::UUID& myID) override;
  bool setLocation(const std::string& location);

  /// Indicate whether the resource is in sync with its location.
  ///
  /// Resources that have a non-empty location and are identical to
  /// the data stored at location are clean. All other resources are dirty.
  ///
  /// Operations that are write-operators (i.e., not read-only) should mark
  /// resources as modified. Saving a resource using its metadata's write
  /// method will mark the resource as clean. Loading a resource using
  /// its metadata's read method should return a clean resource.
  bool clean() const { return m_clean; }
  void setClean(bool state = true) { m_clean = state; }

  /// Resources that are managed have a non-null pointer to their manager.
  ManagerPtr manager() const { return m_manager.lock(); }

  /// given a resource component's UUID, return the resource component.
  virtual ComponentPtr find(const smtk::common::UUID& compId) const = 0;

  /// given a std::string describing a query, return a functor for performing
  /// the query (accepts component as input, returns true if the component
  /// satisfies the query parameters).
  virtual std::function<bool(const ComponentPtr&)> queryOperation(const std::string&) const = 0;

  /// visit all components in a resource.
  virtual void visit(std::function<void(const ComponentPtr&)>& v) const = 0;

  /// given a a std::string describing a query, return a set of components that
  /// satisfy the query criteria.
  ComponentSet find(const std::string& queryString) const;

  Links& links() { return m_links; }
  const Links& links() const { return m_links; }

  /// classes that are granted permission to the key may retrieve the resource's
  /// lock.
  Lock& lock(Key()) const { return m_lock; }

private:
  // Derived resources should inherit
  // smtk::resource::DerivedFrom<Self, smtk::resource::Resource>. Resource's
  // constructors are declared private to enforce this relationship.
  Resource(const smtk::common::UUID&, ManagerPtr manager = nullptr);
  Resource(ManagerPtr manager = nullptr);

  smtk::common::UUID m_id;
  std::string m_location;
  /// True when m_location is in sync with this instance.
  bool m_clean;

  Links m_links;
  mutable Lock m_lock;

  WeakManagerPtr m_manager;
};
}
}

#endif // smtk_resource_Resource_h

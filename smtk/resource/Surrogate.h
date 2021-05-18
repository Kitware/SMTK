//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Surrogate_h
#define smtk_resource_Surrogate_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include <memory>
#include <string>

namespace smtk
{
namespace resource
{

class Component;
class Manager;
class Resource;

/// A resource Surrogate is a stand-in for an actual resource. It is used to
/// represent resources that are not loaded into memory.
class SMTKCORE_EXPORT Surrogate
{
  typedef std::size_t Index;

public:
  /// Constructor for unresolved resource.
  Surrogate(const Index&, const std::string&, const smtk::common::UUID&, const std::string&);

  /// Constructor for resolved resource
  Surrogate(const ResourcePtr&);

  virtual ~Surrogate() = default;

  std::string typeName() const { return m_typeName; }
  Index index() const { return m_index; }
  const smtk::common::UUID& id() const;
  const std::string& location() const;

  /// Return the resource for which the instance is a surrogate.
  ResourcePtr resource() const;

  /// Given a resource component's UUID, return the resource component.
  ComponentPtr find(const smtk::common::UUID&) const;

  /// Load the resource (set m_Resource) using the input resource manager.
  /// Return true if the resource is successfully loaded.
  bool fetch(const ManagerPtr&) const;

  /// Set the resource (set m_Resource) using the input resource. Return true
  /// if the resource is successfully set.
  bool resolve(const ResourcePtr&) const;

private:
  Index m_index;
  std::string m_typeName;
  smtk::common::UUID m_id;
  std::string m_location;

  mutable WeakResourcePtr m_resource;
};
} // namespace resource
} // namespace smtk

#endif // smtk_resource_Surrogate_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Metadata_h
#define smtk_resource_Metadata_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/resource/Resource.h"

#include <functional>
#include <set>
#include <string>

namespace smtk
{
namespace resource
{
class Manager;

/// Resources are registered to a resource manager at runtime with an instance
/// of smtk::resource::Metadata. Instances of this class must provide
/// <uniqueName>, a unique (to the manager) string used to describe the resource
/// within the manager. They may also provide functors for the creation and
/// serialization/deserialization of the resource to/from disk (<create>,
/// <write> and <read>, respectively).
class Metadata
{
  friend class Manager;

public:
  Metadata(const std::string& uniqueName)
    : m_uniqueName(uniqueName)
    , m_index(typeid(smtk::resource::Resource))
  {
  }

  Metadata(const std::string& uniqueName, Resource::Index index)
    : m_uniqueName(uniqueName)
    , m_index(index)
  {
  }

  const std::string& uniqueName() const { return m_uniqueName; }
  const Resource::Index& index() const { return m_index; }

  std::function<ResourcePtr(const smtk::common::UUID&)> create = [](
    const smtk::common::UUID&) { return ResourcePtr(); };
  std::function<ResourcePtr(const std::string&)> read = [](
    const std::string&) { return ResourcePtr(); };
  std::function<bool(const ResourcePtr&)> write = [](const ResourcePtr&) { return false; };

private:
  std::string m_uniqueName;
  Resource::Index m_index;
  std::set<Resource::Index> m_associatedIndices;
};
}
}

#endif // smtk_resource_Metadata_h

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
#include "smtk/common/Managers.h"
#include "smtk/common/UUID.h"
#include "smtk/resource/MetadataObserver.h"
#include "smtk/resource/Resource.h"

#include <functional>
#include <set>
#include <string>

namespace smtk
{
namespace resource
{
class Manager;

/// @brief Resources are registered to a resource manager at runtime with an instance
/// of smtk::resource::Metadata.
///
/// Instances of this class must provide
/// \a typeName, a unique (to the manager) string used to describe the resource
/// within the manager. They may also provide functors for the creation and
/// serialization/deserialization of the resource to/from disk (\a create,
/// \a write and \a read, respectively).
/// There are two variants of the read, write, and create methods: one takes a
/// shared-pointer to an smtk::common::Managers instance and the other does not.
/// The former is preferred.
class Metadata
{
  friend class Manager;

public:
  typedef std::function<void(const Metadata&)> Visitor;
  typedef MetadataObserver Observer;
  typedef MetadataObservers Observers;

  Metadata(
    const std::string& typeName,
    Resource::Index index,
    std::set<Resource::Index> parentIndices,
    std::function<
      ResourcePtr(const smtk::common::UUID&, const std::shared_ptr<smtk::common::Managers>&)>
      createFunctor = nullptr,
    std::function<ResourcePtr(const std::string&, const std::shared_ptr<smtk::common::Managers>&)>
      readFunctor = nullptr,
    std::function<bool(const ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)>
      writeFunctor = nullptr)
    : m_typeName(typeName)
    , m_index(index)
    , m_parentIndices(parentIndices)
  {
    // Only assign functors if they are non-null.
    // Otherwise, use the defaults which do nothing
    // but can always be invoked safely.
    if (createFunctor)
    {
      this->create = createFunctor;
    }
    if (readFunctor)
    {
      this->read = readFunctor;
    }
    if (writeFunctor)
    {
      this->write = writeFunctor;
    }
  }

  const std::string& typeName() const { return m_typeName; }
  const Resource::Index& index() const { return m_index; }

  // Resource metadata holds inheritance information for its resource as a set
  // of parent indices.
  bool isOfType(const Resource::Index& index) const
  {
    return m_parentIndices.find(index) != m_parentIndices.end();
  }

  /// A method that can be called to create a resource of the metadata's type.
  ///
  /// Provide a default method that returns a null pointer.
  std::function<ResourcePtr(const smtk::common::UUID&, const shared_ptr<smtk::common::Managers>&)>
    create =
      [this](const smtk::common::UUID& uid, const shared_ptr<smtk::common::Managers>& managers) {
        (void)uid;
        (void)managers;
        return ResourcePtr();
      };
  /// A method that can be called to read a resource of the metadata's type from a file.
  ///
  /// Provide a default that does nothing but return a null pointer.
  std::function<ResourcePtr(const std::string&, const std::shared_ptr<smtk::common::Managers>&)>
    read =
      [this](const std::string& filename, const std::shared_ptr<smtk::common::Managers>& managers) {
        (void)filename;
        (void)managers;
        return ResourcePtr();
      };
  /// A method that can be called to write a resource of the metadata's type.
  ///
  /// Provide a default that does nothing and always returns false.
  std::function<bool(const ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)> write =
    [this](const ResourcePtr& resource, const std::shared_ptr<smtk::common::Managers>& managers) {
      (void)resource;
      (void)managers;
      return false;
    };

private:
  std::string m_typeName;
  Resource::Index m_index;
  std::set<Resource::Index> m_parentIndices;
};
} // namespace resource
} // namespace smtk

#endif // smtk_resource_Metadata_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Metadata_h
#define smtk_project_Metadata_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/project/MetadataObserver.h"
#include "smtk/project/Project.h"

#include <set>
#include <string>

namespace smtk
{
namespace project
{

/// Project Metadata contains a Project's typename and type index, a functor for
/// creating instances of the Project, a whitelist of Resources and Operations
/// the Project uses, and a version string.
class SMTKCORE_EXPORT Metadata
{
public:
  using Observer = MetadataObserver;
  using Observers = MetadataObservers;

  Metadata(
    const std::string& typeName,
    Project::Index index,
    std::function<
      ProjectPtr(const smtk::common::UUID&, const std::shared_ptr<smtk::common::Managers>&)>
      createFunctor,
    const std::set<std::string>& resources = std::set<std::string>(),
    const std::set<std::string>& operations = std::set<std::string>(),
    const std::string& version = "0.0.0")
    : m_typeName(typeName)
    , m_index(index)
    , m_resources(resources)
    , m_operations(operations)
    , m_version(version)
  {
    if (createFunctor)
    {
      this->create = createFunctor;
    }
  }

  const std::string& typeName() const { return m_typeName; }
  const Project::Index& index() const { return m_index; }

  std::function<
    ProjectPtr(const smtk::common::UUID&, const std::shared_ptr<smtk::common::Managers>&)>
    create =
      [this](const smtk::common::UUID& uid, const shared_ptr<smtk::common::Managers>& managers) {
        (void)uid;
        (void)managers;
        return ProjectPtr();
      };

  const std::set<std::string>& resources() const { return m_resources; }
  const std::set<std::string>& operations() const { return m_operations; }
  const std::string& version() const { return m_version; }

private:
  std::string m_typeName;
  Project::Index m_index;
  std::set<std::string> m_resources;
  std::set<std::string> m_operations;
  std::string m_version;
};
} // namespace project
} // namespace smtk

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Manager_h
#define smtk_project_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h" // needed?

#include "smtk/common/TypeName.h"
#include "smtk/common/UUID.h"

#include <string>
#include <tuple>
#include <vector>

namespace smtk
{
namespace project
{
class Manager;

/// Internal class representing the persistent data stored for each
/// project resource.
class ResourceInfo
{
public:
  /// User-specified string for labeling resource in UI widgets.
  std::string m_identifier;

  /// The filesystem location for the file, if any, that was imported
  /// to create the resource. Examples include .gen or.exo file for
  /// a model resource, or .sbt file for an attribute resource.
  /// (Future) this will be expanded to encompass URL locations, as
  /// well as multiple locations of the same resource,
  std::string m_importLocation; // use boost::path?

  /// Identifies how the resource is used in the project.
  /// Examples include "default" and "export".
  std::string m_role;

  /// (Future) Consider m_checksum for veryifying that import file
  /// hasn't changed. Might be moot if we store projects as archive
  /// files.

  /// Stores the resource type, maybe from smtk::common::typeName() ?
  std::string m_typeName;

  /// Resource UUID
  smtk::common::UUID m_uuid;
}; // class smtk::project::ResourceInfo

/// A project Manager is responsible for tracking a set of resources
/// used in constructing one or more simulation input decks.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypedefs(smtk::project::Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Assign the resource manager to manage resources used in projects.
  bool registerResourceManager(smtk::resource::ManagerPtr&);

  /// Create new project. Returns operator status
  smtk::attribute::ResourcePtr newProjectTemplate() const;
  int newProject(smtk::attribute::ResourcePtr specification);

  /// Return <isLoaded, projectName, projectPath>
  std::tuple<bool, std::string, std::string> projectStatus() const;

  /// Return project resources
  std::vector<std::project::ResourceInfo> resourceInfos() const;

  // Other project methods
  int saveProject();
  int closeProject();
  int loadProject(const std::string& path); // directory or .cmb-project file
  smtk::attribute::ResourcePtr exportTemplate() const;
  int exportProject(smtk::attribute::ResourcePtr specification);

private:
  Manager();

  /// Resource manager for the project resources.
  /// Question: do we need a Registrar for this?
  smtk::resource::ManagerPtr m_resourceManager;

  /// User-supplied name for the project
  std::string m_projectName;

  /// Filesystem directory where project resources are stored.
  std::string m_projectDirectory;

  /// Array of ResourceInfo objects for each project resource.
  /// These data are stored in a file in the project directory.
  std::vector<ResourceInfo> m_resourceInfoArray;
}; // class smtk::project::Manager

} // namespace project
} // namespace smtk

#endif // smtk_project_Manager_h

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

#include "nlohmann/json.hpp"

#include <string>
#include <tuple>
#include <vector>

using json = nlohmann::json;

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
  /// Resource filename
  std::string m_filename;

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
  //  unsigned int m_checksum;

  /// Stores the resource type, maybe from smtk::common::typeName() ?
  std::string m_typeName;

  /// Resource UUID
  smtk::common::UUID m_uuid;

  json to_json() const
  {
    json j = { { "filename", m_filename }, { "identifier", m_identifier },
      { "importLocation", m_importLocation }, { "role", m_role }, { "typeName", m_typeName },
      { "uuid", m_uuid.toString() } };
    return j;
  }
}; // class smtk::project::ResourceInfo

/// A project Manager is responsible for tracking a set of resources
/// used in constructing one or more simulation input datasets.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypedefs(smtk::project::Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Assign resource & operation managers
  void setManagers(smtk::resource::ManagerPtr&, smtk::operation::ManagerPtr&);

  /// Create project with 2 methods: (i) get spec, then (ii) use spec to create
  smtk::attribute::AttributePtr getProjectSpecification() const;
  int createProject(smtk::attribute::AttributePtr specification);

  /// Return <isLoaded, projectName, projectPath>
  std::tuple<bool, std::string, std::string> getStatus() const;

  /// Return project resources
  std::vector<smtk::project::ResourceInfo> getResourceInfos() const;

  // Other project methods
  int saveProject();
  int closeProject();
  int loadProject(const std::string& path); // directory or .cmb-project file
  smtk::attribute::ResourcePtr getExportTemplate() const;
  int exportProject(smtk::attribute::ResourcePtr specification);

  // Future
  // int addResource(smtk::common::ResourcePtr, const std::string& role, const std::string& identifier);
protected:
  // Load and return NewProject template
  smtk::attribute::ResourcePtr getProjectTemplate() const;

  // Import model file and create new resource
  std::tuple<int, smtk::resource::ResourcePtr> importModel(const std::string& path);

  // Write persistent project data to file
  int writeProjectFile() const;

private:
  Manager();

  /// Resource manager for the project resources.
  smtk::resource::ManagerPtr m_resourceManager;

  /// Operation manager for the project operations.
  smtk::operation::ManagerPtr m_operationManager;

  /// User-supplied name for the project
  std::string m_projectName;

  /// Filesystem directory where project resources are stored.
  std::string m_projectDirectory;

  /// Array of ResourceInfo objects for each project resource.
  /// These data are stored in a file in the project directory.
  std::vector<ResourceInfo> m_resourceInfos;
}; // class smtk::project::Manager

} // namespace project
} // namespace smtk

#endif // smtk_project_Manager_h

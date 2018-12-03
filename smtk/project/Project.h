//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Project_h
#define smtk_project_Project_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include "smtk/project/ProjectInfo.h"

#include <string>
#include <vector>

namespace smtk
{
namespace project
{
class Manager;

/// A project Manager is responsible for tracking a set of smtk
/// resources used in constructing one or more simulation input
/// datasets.
class SMTKCORE_EXPORT Project : smtkEnableSharedPtr(Project)
{
  friend class Manager;

public:
  smtkTypedefs(smtk::project::Project);
  smtkCreateMacro(Project);

  virtual ~Project();

  /// Return project name
  std::string name() const { return this->m_name; }

  /// Return project directory
  std::string directory() const { return this->m_directory; }

  /// Return project resources
  std::vector<smtk::project::ResourceInfo> getResourceInfos() const
  {
    return this->m_resourceInfos;
  }

  /// Return resource of given typename and role
  smtk::resource::ResourcePtr getResourceByRole(
    const std::string& typeName, const std::string& role) const;

  // Future:
  // * method to retrieve export operator, based on location of simulation attributes

protected:
  /// First set of methods are called by (friend class) smtk::project::Manager

  /// Assign resource & operation managers
  void setCoreManagers(smtk::resource::ManagerPtr, smtk::operation::ManagerPtr);

  /// Create project from application-provided specification
  int build(smtk::attribute::AttributePtr specification);

  int save() const;

  int close();

  int open(const std::string& path);

  /// Load project from filesystem
  int load(smtk::project::ProjectInfo& info);

  // Remaining calls are for internal use

  int importModel(const std::string& location, bool copyNativeFile, ResourceInfo& resInfo);

  int importAttributeTemplate(const std::string& location, ResourceInfo& resInfo);

  int writeProjectFile() const;

  int loadResources(const std::string& path);

  /// Resource manager for the project resources.
  smtk::resource::ManagerPtr m_resourceManager;

  /// Operation manager for the project operations.
  smtk::operation::ManagerPtr m_operationManager;

  /// User-supplied name for the project
  std::string m_name;

  /// Filesystem directory where project resources are stored.
  std::string m_directory;

  /// Array of ResourceInfo objects for each project resource.
  /// These data are stored in a file in the project directory.
  std::vector<ResourceInfo> m_resourceInfos;

private:
  Project();
}; // class smtk::project::Project

} // namespace project
} // namespace smtk

#endif // smtk_project_Project_h

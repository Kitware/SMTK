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
#include "smtk/SystemConfig.h"

#include <string>
#include <tuple>
#include <vector>

namespace smtk
{
namespace project
{
class Project;

/// A project Manager is responsible for tracking a set of resources
/// used in constructing one or more simulation input datasets.
/// The current implementation only supports one project being "open" at a time.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypedefs(smtk::project::Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Assign resource & operation managers
  void setCoreManagers(smtk::resource::ManagerPtr&, smtk::operation::ManagerPtr&);

  /// Create project with 2 methods: (i) get spec, then (ii) use spec to create
  smtk::attribute::AttributePtr getProjectSpecification() const;

  /// Create project, returning outcome and project pointer
  /// Outcome uses smtk::operation::Operation::Outcome enum, where 3 == SUCCEEDED
  std::tuple<int, ProjectPtr> createProject(smtk::attribute::AttributePtr specification);

  /// Return project instance
  ProjectPtr getCurrentProject() const { return this->m_project; }

  /// Write project resources & metadata to the filesystem
  int saveProject();

  /// Close the project resources
  int closeProject();

  /// Open a new project from the filesystem. Returns outcome and project pointer
  /// The path argument can be set to either the project directory or the .cmbproject
  /// contained in the project directory.
  std::tuple<int, ProjectPtr> openProject(const std::string& path);

  // Future:
  // * method to add additional resources to the current project
  // * support for multiple projects (or multiple project managers?)
  // * shared resources between projects?
protected:
  // Load and return NewProject template
  smtk::attribute::ResourcePtr getProjectTemplate() const;

private:
  Manager();

  /// Resource manager for the project resources.
  smtk::resource::ManagerPtr m_resourceManager;

  /// Operation manager for the project operations.
  smtk::operation::ManagerPtr m_operationManager;

  /// Current project
  smtk::project::ProjectPtr m_project;
}; // class smtk::project::Manager

} // namespace project
} // namespace smtk

#endif // smtk_project_Manager_h

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

#include "smtk/io/Logger.h"
#include "smtk/project/ProjectDescriptor.h"
#include "smtk/project/ResourceDescriptor.h"

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

  virtual ~Project();

  /// Return project name
  std::string name() const { return m_name; }

  /// Return project directory
  std::string directory() const { return m_directory; }

  /// Return project resources
  std::vector<smtk::resource::ResourcePtr> getResources() const;

  // Future:
  // * methods to add/remove project resources
protected:
  /// First set of methods are called by (friend class) smtk::project::Manager
  smtkCreateMacro(Project);

  /// Assign resource & operation managers
  void setCoreManagers(smtk::resource::ManagerPtr, smtk::operation::ManagerPtr);

  /// Create project from application-provided specification
  bool build(smtk::attribute::AttributePtr specification,
    smtk::io::Logger& logger = smtk::io::Logger::instance(), bool replaceExistingDirectory = false);

  bool save(smtk::io::Logger& logger = smtk::io::Logger::instance()) const;

  bool close();

  /// Load project from filesystem
  bool open(const std::string& path, smtk::io::Logger& logger = smtk::io::Logger::instance());

  // Remaining calls are for internal use

  bool importModel(const std::string& location, bool copyNativeFile, ResourceDescriptor& descriptor,
    smtk::io::Logger& logger = smtk::io::Logger::instance());

  bool importAttributeTemplate(const std::string& location, ResourceDescriptor& descriptor,
    smtk::io::Logger& logger = smtk::io::Logger::instance());

  bool writeProjectFile(smtk::io::Logger& logger = smtk::io::Logger::instance()) const;

  bool loadResources(
    const std::string& path, smtk::io::Logger& logger = smtk::io::Logger::instance());

  /// Return export operator
  /// If reset flag is true, will create new operator in order to
  /// reset contents to their default values.
  smtk::operation::OperationPtr getExportOperator(
    smtk::io::Logger& logger = smtk::io::Logger::instance(), bool reset = false);

  void releaseExportOperator();

  /// Resource manager for the project resources.
  smtk::resource::WeakManagerPtr m_resourceManager;

  /// Operation manager for the project operations.
  smtk::operation::WeakManagerPtr m_operationManager;

  /// User-supplied name for the project
  std::string m_name;

  /// Filesystem directory where project resources are stored.
  std::string m_directory;

  /// Array of ResourceDescriptor objects for each project resource.
  /// These data are stored in a file in the project directory.
  std::vector<ResourceDescriptor> m_resourceDescriptors;

  /// Export operator (cached)
  smtk::operation::OperationPtr m_exportOperator;
  std::string m_exportOperatorUniqueName;

private:
  Project();
}; // class smtk::project::Project

} // namespace project
} // namespace smtk

#endif // smtk_project_Project_h

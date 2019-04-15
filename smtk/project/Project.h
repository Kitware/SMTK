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
#include "smtk/resource/Manager.h"

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

  /// Return simulation code this project targets
  std::string simulationCode() const { return m_simulationCode; }

  /// Return project name
  std::string name() const { return m_name; }

  /// Return project directory
  std::string directory() const { return m_directory; }

  /// Return project resources
  std::vector<smtk::resource::ResourcePtr> resources() const;

  /// Return resource "import location", which is the location in the
  /// file system that was imported to create the resource. The return
  /// string might be empty (unknown).
  std::string importLocation(smtk::resource::ResourcePtr res) const;

  /// Return resource of specified type and identifier
  template <typename ResourceType>
  smtk::shared_ptr<ResourceType> findResource(const std::string& identifier) const;

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

  bool importModels(const smtk::attribute::AttributePtr specification, smtk::io::Logger& logger);

  bool importModel(const std::string& location, bool copyNativeFile, ResourceDescriptor& descriptor,
    bool useVtkSession, smtk::io::Logger& logger = smtk::io::Logger::instance());

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

  bool populateExportOperator(smtk::operation::OperationPtr exportOp,
    smtk::io::Logger& logger = smtk::io::Logger::instance()) const;

  void releaseExportOperator();

  /// Resource manager for the project resources.
  smtk::resource::WeakManagerPtr m_resourceManager;

  /// Operation manager for the project operations.
  smtk::operation::WeakManagerPtr m_operationManager;

  /// Target simulation code, e.g., ACE3P, OpenFOAM, Truchas.
  /// The convention is to use the name of the folder in the
  /// simulation-workflows repository, converted to lower case,
  /// e.g. ace3p, openfoam, truchas.
  std::string m_simulationCode;

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

template <typename ResourceType>
smtk::shared_ptr<ResourceType> Project::findResource(const std::string& identifier) const
{
  auto resManager = m_resourceManager.lock();
  if (!resManager)
  {
    return nullptr;
  }

  // Traverse resource descriptors
  for (const auto& rd : m_resourceDescriptors)
  {
    if (rd.m_identifier == identifier)
    {
      smtk::resource::ResourcePtr resource = resManager->get(rd.m_uuid);
      if ((resource != nullptr) && (resource->isOfType<ResourceType>()))
      {
        return smtk::static_pointer_cast<ResourceType>(resource);
      }
    } // if (identifier match)
  }   // for (resource descriptors)

  // (Resource was not found)
  return nullptr;
}

} // namespace project
} // namespace smtk

#endif // smtk_project_Project_h

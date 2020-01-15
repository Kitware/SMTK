//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_old_ProjectDescriptor_h
#define smtk_project_old_ProjectDescriptor_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/project/old/ResourceDescriptor.h"

#include <string>
#include <vector>

namespace smtk
{
namespace project
{
namespace old
{
/// Class representing the persistent data stored for a project.
class SMTKCORE_EXPORT ProjectDescriptor
{
public:
  /// Simulation code identifier, e.g. ace3p, openfoam, truchas.
  std::string m_simulationCode;

  /// User-supplied name for the project.
  std::string m_name;

  /// Filesystem directory where project resources are stored.
  std::string m_directory;

  /// Array descriptor objects, one for each project resource.
  std::vector<ResourceDescriptor> m_resourceDescriptors;

  // (Future) One or more analysis descriptors
  // std::vector<AnalysisDescriptor> m_analysisDescriptors;
};
} // namespace old
} // namespace project
} // namespace smtk

#endif // smtk_project_old_ProjectDescriptor_h

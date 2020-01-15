//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_old_ResourceDescriptor_h
#define smtk_project_old_ResourceDescriptor_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include <string>
#include <vector>

namespace smtk
{
namespace project
{
namespace old
{
/// Class representing the persistent data stored for each
/// resource used in a project. Primarily for internal use.
class ResourceDescriptor
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
  std::string m_importLocation;

  /// (Future) Checksum for veryifying file integrity.
  //  unsigned int m_checksum;

  /// Stores the resource type, as the string returned from smtk::resource::typeName()
  std::string m_typeName;

  /// Resource UUID
  smtk::common::UUID m_uuid;
}; // class smtk::project::ResourceDescriptor
} // namespace old
} // namespace project
} // namespace smtk

#endif // smtk_project_old_ResourceDescriptor_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_ReadMesh_h
#define __smtk_io_ReadMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <string>
#include <vector>

namespace smtk
{
namespace io
{

/**\brief Read an entire SMTK mesh resource from a file, or just sub-sections
  *
  */
class SMTKCORE_EXPORT ReadMesh
{
public:
  ReadMesh();
  ~ReadMesh();

  ReadMesh& operator=(const ReadMesh&) = delete;
  ReadMesh(const ReadMesh&) = delete;

  static std::vector<smtk::io::mesh::MeshIOPtr>& SupportedIOTypes();

  static bool ExtensionIsSupported(const std::string& ext);

  //Load the domain sets from a moab data file as a new resource into the
  //given manager.
  smtk::mesh::ResourcePtr operator()(
    const std::string& filePath,
    const smtk::mesh::InterfacePtr& interface,
    mesh::Subset subset = mesh::Subset::EntireResource) const;
  bool operator()(
    const std::string& filePath,
    smtk::mesh::ResourcePtr resource,
    mesh::Subset subset = mesh::Subset::EntireResource) const;
};

SMTKCORE_EXPORT smtk::mesh::ResourcePtr readMesh(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface,
  mesh::Subset subset = mesh::Subset::EntireResource);
// Explicit functions for each subset type
SMTKCORE_EXPORT smtk::mesh::ResourcePtr readEntireResource(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface);
SMTKCORE_EXPORT smtk::mesh::ResourcePtr readDomain(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface);
SMTKCORE_EXPORT smtk::mesh::ResourcePtr readDirichlet(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface);
SMTKCORE_EXPORT smtk::mesh::ResourcePtr readNeumann(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface);

SMTKCORE_EXPORT bool readMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource,
  mesh::Subset subset = mesh::Subset::EntireResource);
// Explicit functions for each subset type
SMTKCORE_EXPORT bool readEntireResource(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool readDomain(const std::string& filePath, smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool readDirichlet(const std::string& filePath, smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool readNeumann(const std::string& filePath, smtk::mesh::ResourcePtr resource);
} // namespace io
} // namespace smtk

#endif

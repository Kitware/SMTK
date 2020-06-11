//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/mesh/MeshIOMoab.h"
#include "smtk/io/mesh/MeshIO.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/moab/Readers.h"
#include "smtk/mesh/moab/Writers.h"

namespace smtk
{
namespace io
{
namespace mesh
{

MeshIOMoab::MeshIOMoab()
{
  this->Formats.emplace_back("MOAB File", std::vector<std::string>({ ".h5m", ".mhdf" }),
    Format::Import | Format::Export | Format::Read | Format::Write);
  this->Formats.emplace_back("Exodus File via MOAB",
    std::vector<std::string>({ ".ex2", ".exo", ".exoII", ".exo2", ".e", ".g", ".gen" }),
    Format::Import | Format::Export | Format::Read | Format::Write);
  this->Formats.emplace_back(
    "SLAC File via MOAB", std::vector<std::string>({ ".slac" }), Format::Import | Format::Export);
  this->Formats.emplace_back(
    "General Mesh Viewer", std::vector<std::string>({ ".gmv" }), Format::Import | Format::Export);
  this->Formats.emplace_back(
    "ANSYS File", std::vector<std::string>({ ".ans" }), Format::Import | Format::Export);
  this->Formats.emplace_back(
    "Gmsh File", std::vector<std::string>({ ".msh", ".gmsh" }), Format::Import | Format::Export);
#ifdef MOAB_IMPORT_STL
  // An update to MOAB's lastest master caused the stl importer to fail. Until
  // this is fixed, we temporarily disable MOAB's stl reader (we still have
  // VTK's stl reader, if VTK is enabled).
  this->Formats.push_back(Format("Stereolithography File", std::vector<std::string>({ ".stl" }),
    Format::Import | Format::Export));
#endif
  this->Formats.emplace_back(
    "Wavefront OBJ File", std::vector<std::string>({ ".obj" }), Format::Import | Format::Export);
}

smtk::mesh::ResourcePtr MeshIOMoab::importMesh(const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface, const std::string& /*unused*/) const
{
  return this->read(filePath, interface, Subset::EntireResource);
}

bool MeshIOMoab::importMesh(const std::string& filePath, smtk::mesh::ResourcePtr resource,
  const std::string& /*unused*/) const
{
  return this->read(filePath, resource, Subset::EntireResource);
}

bool MeshIOMoab::exportMesh(const std::string& filePath, smtk::mesh::ResourcePtr resource) const
{
  return this->write(filePath, resource, Subset::EntireResource);
}

//TODO:
// bool MeshIOMoab::exportMesh( const std::string& filePath,
//                              smtk::mesh::ResourcePtr resource,
//                              smtk::model::ResourcePtr resource,
//                              const std::string& modelPropertyName ) const
// {

// }

smtk::mesh::ResourcePtr MeshIOMoab::read(
  const std::string& filePath, const smtk::mesh::InterfacePtr& /*unused*/, Subset subset) const
{
  smtk::mesh::ResourcePtr resource;

  switch (subset)
  {
    case Subset::EntireResource:
      resource = smtk::mesh::moab::read(filePath);
      break;
    case Subset::OnlyDomain:
      resource = smtk::mesh::moab::read_domain(filePath);
      break;
    case Subset::OnlyDirichlet:
      resource = smtk::mesh::moab::read_dirichlet(filePath);
      break;
    case Subset::OnlyNeumann:
      resource = smtk::mesh::moab::read_neumann(filePath);
      break;
    default:
      resource = smtk::mesh::Resource::create();
  }
  return resource;
}

bool MeshIOMoab::read(
  const std::string& filePath, smtk::mesh::ResourcePtr resource, Subset subset) const
{
  bool result;

  switch (subset)
  {
    case Subset::EntireResource:
      result = smtk::mesh::moab::import(filePath, resource);
      break;
    case Subset::OnlyDomain:
      result = smtk::mesh::moab::import_domain(filePath, resource);
      break;
    case Subset::OnlyDirichlet:
      result = smtk::mesh::moab::import_dirichlet(filePath, resource);
      break;
    case Subset::OnlyNeumann:
      result = smtk::mesh::moab::import_neumann(filePath, resource);
      break;
    default:
      result = false;
  }
  return result;
}

bool MeshIOMoab::write(
  const std::string& filePath, smtk::mesh::ResourcePtr resource, Subset subset) const
{
  switch (subset)
  {
    case Subset::EntireResource:
      return smtk::mesh::moab::write(filePath, resource);
    case Subset::OnlyDomain:
      return smtk::mesh::moab::write_domain(filePath, resource);
    case Subset::OnlyDirichlet:
      return smtk::mesh::moab::write_dirichlet(filePath, resource);
    case Subset::OnlyNeumann:
      return smtk::mesh::moab::write_neumann(filePath, resource);
    default:
      return false;
  }
}

bool MeshIOMoab::write(smtk::mesh::ResourcePtr resource, Subset subset) const
{
  if (resource->writeLocation().empty())
  { //require a file location
    return false;
  }

  return this->write(resource->writeLocation().absolutePath(), resource, subset);
}
}
}
}

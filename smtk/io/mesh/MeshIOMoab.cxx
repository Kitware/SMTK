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
  : MeshIO()
{
  this->Formats.push_back(Format("moab", std::vector<std::string>({ ".h5m", ".mhdf" }),
    Format::Import | Format::Export | Format::Read | Format::Write));
  this->Formats.push_back(
    Format("exodus", std::vector<std::string>({ ".ex2", ".exo", ".exoII", ".exo2", ".g", ".gen" }),
      Format::Import | Format::Export | Format::Read | Format::Write));
  this->Formats.push_back(
    Format("slac", std::vector<std::string>({ ".slac" }), Format::Import | Format::Export));
  this->Formats.push_back(Format(
    "general mesh viewer", std::vector<std::string>({ ".gmv" }), Format::Import | Format::Export));
  this->Formats.push_back(
    Format("ansys", std::vector<std::string>({ ".ans" }), Format::Import | Format::Export));
  this->Formats.push_back(
    Format("gmsh", std::vector<std::string>({ ".msh", ".gmsh" }), Format::Import | Format::Export));
  this->Formats.push_back(
    Format("stl", std::vector<std::string>({ ".stl" }), Format::Import | Format::Export));
}

smtk::mesh::ResourcePtr MeshIOMoab::importMesh(
  const std::string& filePath, const smtk::mesh::InterfacePtr& interface, const std::string&) const
{
  return this->read(filePath, interface, Subset::EntireResource);
}

bool MeshIOMoab::importMesh(
  const std::string& filePath, smtk::mesh::ResourcePtr resource, const std::string&) const
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
  const std::string& filePath, const smtk::mesh::InterfacePtr&, Subset subset) const
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

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

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/mesh/moab/Readers.h"
#include "smtk/mesh/moab/Writers.h"

namespace smtk {
  namespace io {
namespace mesh {

MeshIOMoab::MeshIOMoab() : MeshIO()
{
  this->Formats.push_back(
    Format("moab",
           std::vector<std::string>({ ".h5m", ".mhdf" }),
           Format::Import|Format::Export|Format::Read|Format::Write));
  this->Formats.push_back(
    Format("exodus",
           std::vector<std::string>({ ".exo", ".exoII", ".exo2", ".g", ".gen"}),
           Format::Import|Format::Export|Format::Read|Format::Write));
  this->Formats.push_back(
    Format("vtk",
           std::vector<std::string>({ ".vtk" }),
           Format::Import|Format::Export));
  this->Formats.push_back(
    Format("slac",
           std::vector<std::string>({ ".slac" }),
           Format::Import|Format::Export));
  this->Formats.push_back(
    Format("general mesh viewer",
           std::vector<std::string>({ ".gmv" }),
           Format::Import|Format::Export));
  this->Formats.push_back(
    Format("ansys",
           std::vector<std::string>({ ".ans" }),
           Format::Import|Format::Export));
  this->Formats.push_back(
    Format("gmsh",
           std::vector<std::string>({ ".msh", ".gmsh" }),
           Format::Import|Format::Export));
  this->Formats.push_back(
    Format("stl",
           std::vector<std::string>({ ".stl" }),
           Format::Import|Format::Export));
}

smtk::mesh::CollectionPtr
MeshIOMoab::importMesh( const std::string& filePath,
                        smtk::mesh::ManagerPtr& manager ) const
{
  return this->read( filePath, manager, Subset::EntireCollection );
}

bool MeshIOMoab::importMesh( const std::string& filePath,
                             smtk::mesh::CollectionPtr collection ) const
{
  return this->read( filePath, collection, Subset::EntireCollection );
}

bool MeshIOMoab::exportMesh( const std::string& filePath,
                             smtk::mesh::CollectionPtr collection) const
{
  return this->write( filePath, collection, Subset::EntireCollection );
}

//TODO:
// bool MeshIOMoab::exportMesh( const std::string& filePath,
//                              smtk::mesh::CollectionPtr collection,
//                              smtk::model::ManagerPtr manager,
//                              const std::string& modelPropertyName ) const
// {

// }

smtk::mesh::CollectionPtr MeshIOMoab::read( const std::string& filePath,
                                            smtk::mesh::ManagerPtr& manager,
                                            Subset subset ) const
{
  smtk::mesh::CollectionPtr collection;

  switch ( subset )
    {
    case Subset::EntireCollection:
      collection = smtk::mesh::moab::read(filePath, manager);
      break;
    case Subset::OnlyDomain:
      collection = smtk::mesh::moab::read_domain(filePath, manager);
      break;
    case Subset::OnlyDirichlet:
      collection = smtk::mesh::moab::read_dirichlet(filePath, manager);
      break;
    case Subset::OnlyNeumann:
      collection = smtk::mesh::moab::read_neumann(filePath, manager);
      break;
    default:
      collection = smtk::mesh::Collection::create();
    }

  collection->readLocation(filePath);
  return collection;
}

bool MeshIOMoab::read( const std::string& filePath,
                       smtk::mesh::CollectionPtr collection,
                       Subset subset ) const
{
  bool result;

  switch ( subset )
    {
    case Subset::EntireCollection:
      result = smtk::mesh::moab::import(filePath, collection);
      break;
    case Subset::OnlyDomain:
      result = smtk::mesh::moab::import_domain(filePath, collection);
      break;
    case Subset::OnlyDirichlet:
      result = smtk::mesh::moab::import_dirichlet(filePath, collection);
      break;
    case Subset::OnlyNeumann:
      result = smtk::mesh::moab::import_neumann(filePath, collection);
      break;
    default:
      result = false;
    }

  if (result && collection->readLocation().empty())
    { //if no read location is associated with this file, specify one
    collection->readLocation(filePath);
    }
  return result;
}

bool MeshIOMoab::write( const std::string& filePath,
                        smtk::mesh::CollectionPtr collection,
                        Subset subset ) const
{
  switch ( subset )
    {
    case Subset::EntireCollection:
      return smtk::mesh::moab::write(filePath, collection);
    case Subset::OnlyDomain:
      return smtk::mesh::moab::write_domain(filePath, collection);
    case Subset::OnlyDirichlet:
      return smtk::mesh::moab::write_dirichlet(filePath, collection);
    case Subset::OnlyNeumann:
      return smtk::mesh::moab::write_neumann(filePath, collection);
    default:
      return false;
    }
}

bool MeshIOMoab::write( smtk::mesh::CollectionPtr collection,
                        Subset subset ) const
{
  if( collection->writeLocation().empty() )
  { //require a file location
    return false;
  }

  return this->write( collection->writeLocation().absolutePath(),
                       collection, subset );
}

}
}
}

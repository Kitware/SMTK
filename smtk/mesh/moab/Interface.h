//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================


#ifndef __smtk_mesh_moab_Interface_h
#define __smtk_mesh_moab_Interface_h

#include "smtk/PublicPointerDefs.h"
#include "moab/Interface.hpp"

namespace smtk {
namespace mesh {
namespace moab
{

//Interface is defined in PublicPointerDefs as a typedef to moab::interface
//We don't inherit from moab::interface since it is an abstract class
//Requires the CollectionPtr to not be NULL
const smtk::mesh::moab::InterfacePtr& extractInterface(smtk::mesh::CollectionPtr c);

//construct an empty interface instance, this is properly connected
//to a moab database
smtk::mesh::moab::InterfacePtr make_interface();

//construct an interface to a given file. will load all meshes inside the
//file. If the file given fails to load we will return a invalid InterfacePtr
smtk::mesh::moab::InterfacePtr make_interface(const std::string& path);

//construct an interface to a given file. will load only meshes which are in
//the boundary set.
//file. If the file given fails to load we will return a invalid InterfacePtr
smtk::mesh::moab::InterfacePtr make_boundary_interface(const std::string& path);

//construct an interface to a given file. will load only meshes which are in
//the neumann set.
//file. If the file given fails to load we will return a invalid InterfacePtr
smtk::mesh::moab::InterfacePtr make_neumann_interface(const std::string& path);

//construct an interface to a given file. will load only meshes which are in
//the dirichlet set.
//file. If the file given fails to load we will return a invalid InterfacePtr
smtk::mesh::moab::InterfacePtr make_dirichlet_interface(const std::string& path);


//Import everything in a file into an existing collection.
bool import(const std::string& path, const smtk::mesh::CollectionPtr& c);

//Import all the boundary sets in a file into an existing collection
bool import_boundary(const std::string& path, const smtk::mesh::CollectionPtr& c);

//Import all the neumann sets in a file into an existing collection
bool import_neumann(const std::string& path, const smtk::mesh::CollectionPtr& c);

//Import all the dirichlet sets in a file into an existing collection
bool import_dirichlet(const std::string& path, const smtk::mesh::CollectionPtr& c);


//Write everything in a file into an existing collection.
bool write(const std::string& path, const smtk::mesh::CollectionPtr& c);

//Write all the boundary sets in a file into an existing collection
bool write_boundary(const std::string& path, const smtk::mesh::CollectionPtr& c);

//Write all the neumann sets in a file into an existing collection
bool write_neumann(const std::string& path, const smtk::mesh::CollectionPtr& c);

//Write all the dirichlet sets in a file into an existing collection
bool write_dirichlet(const std::string& path, const smtk::mesh::CollectionPtr& c);


}
}
}

#endif

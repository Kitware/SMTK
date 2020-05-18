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

#ifndef __smtk_mesh_moab_Readers_h
#define __smtk_mesh_moab_Readers_h

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace mesh
{
namespace moab
{

//construct an interface to a given file. will load all meshes inside the
//file. If the file given fails to load we will return a invalid resource
smtk::mesh::ResourcePtr read(const std::string& path);

//construct an interface to a given file. will load only meshes which are in
//the material set.
//file. If the file given fails to load we will return a invalid resource
smtk::mesh::ResourcePtr read_domain(const std::string& path);

//construct an interface to a given file. will load only meshes which are in
//the neumann set.
//file. If the file given fails to load we will return a invalid resource
smtk::mesh::ResourcePtr read_neumann(const std::string& path);

//construct an interface to a given file. will load only meshes which are in
//the dirichlet set.
//file. If the file given fails to load we will return a invalid resource
smtk::mesh::ResourcePtr read_dirichlet(const std::string& path);

//Import everything in a file into an existing resource.
bool import(const std::string& path, const smtk::mesh::ResourcePtr& resource);

//Import all the material sets in a file into an existing resource
bool import_domain(const std::string& path, const smtk::mesh::ResourcePtr& resource);

//Import all the neumann sets in a file into an existing resource
bool import_neumann(const std::string& path, const smtk::mesh::ResourcePtr& resource);

//Import all the dirichlet sets in a file into an existing resource
bool import_dirichlet(const std::string& path, const smtk::mesh::ResourcePtr& resource);
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif

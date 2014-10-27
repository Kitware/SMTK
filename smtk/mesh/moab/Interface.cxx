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


#include "smtk/mesh/moab/Interface.h"
#include "smtk/mesh/Collection.h"
#include "moab/Core.hpp"

namespace smtk {
namespace mesh {
namespace moab {

//construct an empty interface instance
smtk::mesh::moab::InterfacePtr make_interface()
{
  smtk::mesh::moab::InterfacePtr moab( new ::moab::Core() );
  return moab;
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::moab::InterfacePtr make_interface(const std::string& path)
{
  //Core is a fully implemented moab::Interface
  smtk::mesh::moab::InterfacePtr moab( new ::moab::Core() );
  ::moab::ErrorCode err = moab->load_file( path.c_str() );
  if(err != ::moab::MB_SUCCESS)
    { //if the load failed return a null shared_ptr to mark this failed
    moab.reset();
    }
  return moab;
}

//extract the interfacPtr from a collection
const smtk::mesh::moab::InterfacePtr& extractInterface(smtk::mesh::CollectionPtr c)
{
  return c->extractInterface();
}


}
}
}
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
#include "moab/FileOptions.hpp"
#include "moab/ReaderIface.hpp"

namespace smtk {
namespace mesh {
namespace moab {

namespace
{
  template< typename T>
  bool is_valid( const T& t)
  {
    return (t != NULL && t->isValid());
  }


  bool moab_load( const smtk::mesh::moab::InterfacePtr& interface,
                  const std::string& path,
                  const char* subset_name_to_load)
  {
  //currently the moab::interface doesn't allow loading a subset of a file
  //if you don't know any tag values. Our current workaround is to cast
  //to a Core and query for all the tag values, and pass those as the
  //ones we want to load. If we get back no tag values and we wanted
  //to load a subset we fail.
  ::moab::Core* core = dynamic_cast< ::moab::Core* >(interface.get());
  if(!core)
    {
    return false;
    }

  //when we have no subset to load tag_values is empty and
  //dereferencing an empty std::vector is undefined behavior
  std::vector<int> tag_values;
  int* tag_values_ptr = NULL;
  int num_tag_values = 0;

  //if we have a have subset to load, we need to consider have zero values
  //for that tag to be a failure to load, and not to load in the entire file
  if(subset_name_to_load)
    {
    ::moab::ErrorCode tag_err = core->serial_read_tag(path.c_str(),
                                                      subset_name_to_load,
                                                      NULL, //options
                                                      tag_values);

    //this file has no mesh sets that match the given tag so we should
    //fail as trying to load now will bring in the entire mesh, which is wrong
    if(tag_err != ::moab::MB_SUCCESS || tag_values.size() == 0)
      {
      return false;
      }

    tag_values_ptr = &tag_values[0];
    num_tag_values = tag_values.size();
    }

  ::moab::ErrorCode err = interface->load_file( path.c_str(),
                                                NULL, //file set to append to
                                                NULL, //options
                                                subset_name_to_load,
                                                tag_values_ptr,
                                                num_tag_values);
#ifndef NDEBUG
  if(err != ::moab::MB_SUCCESS)
    {
    std::string msg; interface->get_last_error(msg);
    std::cerr << msg << std::endl;
    std::cerr << "failed to load file: " << path << std::endl;
    }
#endif

  return err == ::moab::MB_SUCCESS;
  }

  //requires that interface is not a null shared ptr
  smtk::mesh::moab::InterfacePtr load_file( smtk::mesh::moab::InterfacePtr interface,
                                            const std::string& path,
                                            const char* tag_name = NULL)
  {
  const bool loaded = moab_load(interface, path, tag_name);
  if(!loaded)
    {
    //if we have gotten to here we failed to load so we need to return a
    //null shared_ptr to mark this as failed
    interface.reset();
    }
  return interface;
  }

  //requires that interface is not a null shared ptr
  bool append_file( const smtk::mesh::moab::InterfacePtr& interface,
                    const std::string& path,
                    const char* tag_name = NULL)
  {
  return moab_load(interface, path, tag_name);
  }

  //requires that interface is not a null shared ptr
  bool write_file( const smtk::mesh::moab::InterfacePtr& interface,
                   const std::string& path,
                   const char* tag_name = NULL)
  {//tag_name which is NULL loads in all meshes
  return false;
  }
}

//extract the interfacPtr from a collection
const smtk::mesh::moab::InterfacePtr& extractInterface(smtk::mesh::CollectionPtr c)
{
  return c->extractInterface();
}

//construct an empty interface instance
smtk::mesh::moab::InterfacePtr make_interface()
{
  //Core is a fully implemented moab::Interface
  return smtk::mesh::moab::InterfacePtr( new ::moab::Core() );
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::moab::InterfacePtr make_interface(const std::string& path)
{
  return load_file( make_interface(), path );
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::moab::InterfacePtr make_boundary_interface(const std::string& path)
{
  const std::string tag("BOUNDARY_SET");
  return load_file( make_interface(), path, tag.c_str() );
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::moab::InterfacePtr make_neumann_interface(const std::string& path)
{
  //Core is a fully implemented moab::Interface
  const std::string tag("NEUMANN_SET");
  return load_file( make_interface(), path, tag.c_str() );
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::moab::InterfacePtr make_dirichlet_interface(const std::string& path)
{
  const std::string tag("DIRICHLET_SET");
  return load_file( make_interface(), path, tag.c_str() );
}

//Import everything in a file into an existing collection
bool import(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  return is_valid(c) && append_file( extractInterface(c), path );
}

//Import all the boundary sets in a file into an existing collection
bool import_boundary(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("BOUNDARY_SET");
  return is_valid(c) && append_file( extractInterface(c), path, tag.c_str() );
}

//Import all the neumann sets in a file into an existing collection
bool import_neumann(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("NEUMANN_SET");
  return is_valid(c) && append_file( extractInterface(c), path, tag.c_str() );
}

//Import all the dirichlet sets in a file into an existing collection
bool import_dirichlet(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("DIRICHLET_SET");
  return is_valid(c) && append_file( extractInterface(c), path, tag.c_str() );
}

//Write everything in a file into an existing collection.
bool write(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  return is_valid(c) && write_file( extractInterface(c), path );
}

//Write all the boundary sets in a file into an existing collection
bool write_boundary(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("BOUNDARY_SET");
  return is_valid(c) &&  write_file( extractInterface(c), path, tag.c_str() );
}

//Write all the neumann sets in a file into an existing collection
bool write_neumann(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("BOUNDARY_SET");
  return is_valid(c) && write_file( extractInterface(c), path, tag.c_str() );
}

//Write all the dirichlet sets in a file into an existing collection
bool write_dirichlet(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("BOUNDARY_SET");
  return is_valid(c) && write_file( extractInterface(c), path, tag.c_str() );
}


}
}
}

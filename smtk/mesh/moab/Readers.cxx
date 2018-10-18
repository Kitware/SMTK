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
#include "smtk/mesh/moab/Readers.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/core/Collection.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Core.hpp"
#include "moab/FileOptions.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace mesh
{
namespace moab
{

namespace
{

smtk::mesh::CollectionPtr verifyAndMake(const smtk::mesh::moab::InterfacePtr interface)
{
  if (!interface)
  {
    //create an invalid colection (i.e. one with an invalid id)
    smtk::mesh::CollectionPtr collection = smtk::mesh::Collection::create();
    collection->setId(smtk::common::UUID::null());
    return collection;
  }
  //make a moab specific mesh collection
  return smtk::mesh::Collection::create(interface);
}

template <typename T>
bool is_valid(const T& t)
{
  return (!!t && t->isValid());
}

bool moab_load(const smtk::mesh::moab::InterfacePtr& interface, const std::string& path,
  const char* subset_name_to_load)
{
  //currently the moab::interface doesn't allow loading a subset of a file
  //if you don't know any tag values. Our current workaround is to cast
  //to a Core and query for all the tag values, and pass those as the
  //ones we want to load. If we get back no tag values and we wanted
  //to load a subset we fail.
  ::moab::Interface* m_iface = interface->moabInterface();
  ::moab::Core* core = dynamic_cast< ::moab::Core*>(m_iface);
  if (!core)
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
  if (subset_name_to_load)
  {
    ::moab::ErrorCode tag_err = core->serial_read_tag(path.c_str(), subset_name_to_load,
      NULL, //options
      tag_values);

    //this file has no mesh sets that match the given tag so we should
    //fail as trying to load now will bring in the entire mesh, which is wrong
    if (tag_err != ::moab::MB_SUCCESS || tag_values.size() == 0)
    {
      return false;
    }

    tag_values_ptr = &tag_values[0];
    num_tag_values = static_cast<int>(tag_values.size());
  }

  ::moab::ErrorCode err = m_iface->load_file(path.c_str(),
    NULL, //file set to append to
    NULL, //options
    subset_name_to_load, tag_values_ptr, num_tag_values);
#ifndef NDEBUG
  if (err != ::moab::MB_SUCCESS)
  {
    std::string msg;
    m_iface->get_last_error(msg);
    std::cerr << msg << std::endl;
    std::cerr << "failed to load file: " << path << std::endl;
  }
#endif

  //moab has a concept of "reverse meshes" that are mesh sets that span the same
  //cells and points as an extant mesh set, but are tagged by "NEUSET_SENSE".
  //In smtk we handle this concept at the model level, so we filter out meshsets
  // of this type here.
  {
    ::moab::Range sense_sets;
    ::moab::Tag sense_tag;
    m_iface->tag_get_handle("NEUSET_SENSE", 1, ::moab::MB_TYPE_INTEGER, sense_tag);
    m_iface->get_entities_by_type_and_tag(
      0, ::moab::MBENTITYSET, &sense_tag, NULL, 1, sense_sets, ::moab::Interface::UNION);
    m_iface->delete_entities(sense_sets);
  }

  const bool readFromDisk = (err == ::moab::MB_SUCCESS);
  if (readFromDisk)
  { //if we are loaded from file, we clear the modified flag
    interface->setModifiedState(false);
  }

  return err == ::moab::MB_SUCCESS;
}

//requires that interface is not a null shared ptr
smtk::mesh::moab::InterfacePtr load_file(
  smtk::mesh::moab::InterfacePtr interface, const std::string& path, const char* tag_name = NULL)
{
  const bool loaded = moab_load(interface, path, tag_name);
  if (!loaded)
  {
    //if we have gotten to here we failed to load so we need to return a
    //null shared_ptr to mark this as failed
    interface.reset();
  }
  return interface;
}

//requires that interface is not a null shared ptr
bool append_file(const smtk::mesh::moab::InterfacePtr& interface, const std::string& path,
  const char* tag_name = NULL)
{
  return moab_load(interface, path, tag_name);
}
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::CollectionPtr read(const std::string& path)
{
  return verifyAndMake(load_file(smtk::mesh::moab::make_interface(), path));
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::CollectionPtr read_domain(const std::string& path)
{
  const std::string tag("MATERIAL_SET");
  return verifyAndMake(load_file(smtk::mesh::moab::make_interface(), path, tag.c_str()));
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::CollectionPtr read_neumann(const std::string& path)
{
  //Core is a fully implemented moab::Interface
  const std::string tag("NEUMANN_SET");
  return verifyAndMake(load_file(smtk::mesh::moab::make_interface(), path, tag.c_str()));
}

//construct an interface to a given file. will load all meshes inside the
//file
smtk::mesh::CollectionPtr read_dirichlet(const std::string& path)
{
  const std::string tag("DIRICHLET_SET");
  return verifyAndMake(load_file(smtk::mesh::moab::make_interface(), path, tag.c_str()));
}

//Import everything in a file into an existing collection
bool import(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  return is_valid(c) && append_file(smtk::mesh::moab::extract_interface(c), path);
}

//Import all the domain sets in a file into an existing collection
bool import_domain(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("MATERIAL_SET");
  return is_valid(c) && append_file(smtk::mesh::moab::extract_interface(c), path, tag.c_str());
}

//Import all the neumann sets in a file into an existing collection
bool import_neumann(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("NEUMANN_SET");
  return is_valid(c) && append_file(smtk::mesh::moab::extract_interface(c), path, tag.c_str());
}

//Import all the dirichlet sets in a file into an existing collection
bool import_dirichlet(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("DIRICHLET_SET");
  return is_valid(c) && append_file(smtk::mesh::moab::extract_interface(c), path, tag.c_str());
}
}
}
}

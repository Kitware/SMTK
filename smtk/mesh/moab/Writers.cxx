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
#include "smtk/mesh/moab/Writers.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/Collection.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Interface.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk {
namespace mesh {
namespace moab {

namespace
{
  template< typename T>
  bool is_valid( const T& t)
  {
    return (!!t && t->isValid());
  }

  //--------------------------------------------------------------------------
  bool moab_write( const smtk::mesh::moab::InterfacePtr& interface,
                   const std::string& path,
                   const char* subset_name_to_write)
  {
  ::moab::Interface* m_iface = interface->moabInterface();
  ::moab::ErrorCode err = ::moab::MB_FAILURE;

  if(subset_name_to_write)
    {
    //if we are writing out a subset we first need to find the model
    //entities with the given tag
    ::moab::Tag subsetTag;
    //all our tags are a single integer value
    m_iface->tag_get_handle(subset_name_to_write,
                              1,
                              ::moab::MB_TYPE_INTEGER,
                              subsetTag);

    ::moab::Range setsToSave;
    m_iface->get_entities_by_type_and_tag( m_iface->get_root_set(),
                                           ::moab::MBENTITYSET,
                                           &subsetTag,
                                           NULL,
                                           1,
                                           setsToSave,
                                           ::moab::Interface::UNION);

    if( setsToSave.empty() )
      {
#ifndef NDEBUG
      std::cerr << "failed to write file: " << path <<
                   " no entities with tag: " << subset_name_to_write << std::endl;
#endif
      return false;
      }

    ::moab::Range entsToSave;
    for( ::moab::Range::const_iterator i = setsToSave.begin();
         i != setsToSave.end();
         ++i)
      {
      m_iface->get_entities_by_handle( *i, entsToSave);
      }

    ::moab::Range entitiesToSave = setsToSave;
    entitiesToSave.merge( entsToSave );
    entitiesToSave.insert( 0 );

    //write out just the subset. We let the file extension the user specified
    //determine what writer to use.
    err = m_iface->write_file(path.c_str(),
                              NULL, //explicit writer type
                              NULL, //options
                              entitiesToSave);

    }
  else
    {
    //write out everything. We let the file extension the user specified
    //determine what writer to use.
    err = m_iface->write_file(path.c_str());
    }

#ifndef NDEBUG
  if(err != ::moab::MB_SUCCESS)
    {
    std::string msg; m_iface->get_last_error(msg);
    std::cerr << msg << std::endl;
    std::cerr << "failed to write file: " << path << std::endl;
    }
#endif

  const bool written = (err == ::moab::MB_SUCCESS);
  if(written)
    { //if we have written to file, we clear the modified flag
    interface->setModifiedState(false);
    }
  return written;
  }

  //--------------------------------------------------------------------------
  //requires that interface is not a null shared ptr
  bool write_file( const smtk::mesh::moab::InterfacePtr& interface,
                   const std::string& path,
                   const char* tag_name = NULL)
  {//tag_name which is NULL loads in all meshes
  return moab_write(interface, path, tag_name);
  }
}

//Write everything in a file into an existing collection.
bool write(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  return is_valid(c) && write_file( smtk::mesh::moab::extract_interface(c), path );
}

//Write all the domain sets in a file into an existing collection
bool write_domain(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("MATERIAL_SET");
  return is_valid(c) &&  write_file( smtk::mesh::moab::extract_interface(c), path, tag.c_str() );
}

//Write all the neumann sets in a file into an existing collection
bool write_neumann(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("NEUMANN_SET");
  return is_valid(c) && write_file( smtk::mesh::moab::extract_interface(c), path, tag.c_str() );
}

//Write all the dirichlet sets in a file into an existing collection
bool write_dirichlet(const std::string& path, const smtk::mesh::CollectionPtr& c)
{
  const std::string tag("DIRICHLET_SET");
  return is_valid(c) && write_file( smtk::mesh::moab::extract_interface(c), path, tag.c_str() );
}


}
}
}

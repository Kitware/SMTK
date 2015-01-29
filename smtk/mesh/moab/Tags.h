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

#ifndef __smtk_mesh_moab_Tags_h
#define __smtk_mesh_moab_Tags_h

#ifndef BEING_INCLUDED_BY_INTERFACE_CXX
# error This file can only be included by smtk::mesh::moab::Interface
#endif

#include "MBTagConventions.hpp"

namespace smtk {
namespace mesh {
namespace moab {
namespace tag  {

//these classes should only be included by Interface.cxx

class QueryNameTag
{
  ::moab::Interface* m_iface;
  ::moab::Tag m_tag;
  char m_tagData[32]; //same length as NAME_TAG_SIZE
public:
  QueryNameTag(::moab::Interface* iface):
  m_iface(iface),
  m_tag(),
  m_tagData()
  {
  //populate our tag
  this->m_iface->tag_get_handle(NAME_TAG_NAME,
                                NAME_TAG_SIZE,
                                ::moab::MB_TYPE_OPAQUE,
                                this->m_tag);
  }

  bool fetch_name(const smtk::mesh::Handle& entity)
  {
  ::moab::ErrorCode rval= m_iface->tag_get_data(m_tag,&entity,1,&m_tagData);
  return (rval == ::moab::MB_SUCCESS);
  }

  const char* current_name( ) const { return this->m_tagData; }
};

class QueryDimTag
{
  ::moab::Interface* m_iface;
  ::moab::Tag m_tag;
  int m_dim;
public:
  QueryDimTag(int dimension, ::moab::Interface* iface):
  m_iface(iface),
  m_tag(),
  m_dim( dimension )
  {
  //populate our tag
  this->m_iface->tag_get_handle("GEOM_DIMENSION",
                                1,
                                ::moab::MB_TYPE_INTEGER,
                                this->m_tag);
  }

  const ::moab::Tag* moabTag() const { return &this->m_tag; }
  const ::moab::Tag& moabTagAsRef() const { return this->m_tag; }

  int size() const { return 1; }
  int value() const { return this->m_dim; }
};

}
}
}
}

#endif

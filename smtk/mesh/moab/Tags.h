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


class QueryIntTag
{
  ::moab::Interface* m_iface;
  ::moab::TagInfo* m_tag;
  std::string m_tag_name;
  int m_value;
  void const* tag_v_ptr;
public:
  QueryIntTag(const char* name,
              int value,
              ::moab::Interface* iface)
  {
  this->m_iface = iface;
  this->m_tag_name = std::string(name);
  this->m_value = value;

  //populate our tag
  ::moab::Tag moab_tag;
  this->m_iface->tag_get_handle(this->m_tag_name.c_str(),
                                1,
                                ::moab::MB_TYPE_INTEGER,
                                moab_tag);

  this->m_tag = moab_tag;
  //get the memory location of m_value
  this->tag_v_ptr = &this->m_value;
  }

  ::moab::TagInfo* moabTag() { return this->m_tag; }

  ::moab::TagInfo* const* moabTagPtr() { return &this->m_tag; }
  const void* const* moabTagValuePtr() const { return &this->tag_v_ptr; }

  int size() const { return 1; }
  int value() const { return this->m_value; }
};


class QueryMaterialTag : public QueryIntTag
{
public: QueryMaterialTag(int v, ::moab::Interface* iface):QueryIntTag("MATERIAL_SET",v,iface){}
};

class QueryDirichletTag : public QueryIntTag
{
public: QueryDirichletTag(int v, ::moab::Interface* iface):QueryIntTag("DIRICHLET_SET",v,iface){}
};

class QueryNeumannTag: public QueryIntTag
{
public: QueryNeumannTag(int v, ::moab::Interface* iface):QueryIntTag("NEUMANN_SET",v,iface){}
};

class QueryDimTag: public QueryIntTag
{
public: QueryDimTag(int v, ::moab::Interface* iface):QueryIntTag("GEOM_DIMENSION",v,iface){}
};

}
}
}
}

#endif

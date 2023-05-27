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

#ifndef smtk_mesh_moab_Tags_h
#define smtk_mesh_moab_Tags_h

#ifndef BEING_INCLUDED_BY_INTERFACE_CXX
#error This file can only be included by smtk::mesh::moab::Interface
#endif

#include "smtk/common/UUID.h"

#include "MBTagConventions.hpp"

#include <cstring> // for memcpy (opaque tags)

namespace smtk
{
namespace mesh
{
namespace moab
{
namespace tag
{

//these classes should only be included by Interface.cxx

class QueryNameTag
{
  ::moab::Interface* m_iface;
  ::moab::Tag m_tag;
  char m_tagData[32]; //same length as NAME_TAG_SIZE
public:
//disable warning about elements of array 'm_tagData' will be default initialized
//this is only a warning on msvc, since previously it was broken and wouldn't
//default initialize member arrays
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4351)
#endif
  QueryNameTag(::moab::Interface* iface)
    : m_iface(iface)
  {
    //populate our tag
    m_iface->tag_get_handle(
      NAME_TAG_NAME,
      NAME_TAG_SIZE,
      ::moab::MB_TYPE_OPAQUE,
      m_tag,
      ::moab::MB_TAG_SPARSE | ::moab::MB_TAG_CREAT);
  }
//reset our warnings to the original level
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  bool fetch_name(const smtk::mesh::Handle& entity)
  {
    if (!m_tag)
    {
      return false;
    }

    ::moab::ErrorCode rval = m_iface->tag_get_data(m_tag, &entity, 1, &m_tagData);
    return (rval == ::moab::MB_SUCCESS);
  }

  bool set_name(const smtk::mesh::Handle& entity, const std::string& name)
  {
    if (!m_tag)
    {
      return false;
    }

    strcpy(m_tagData, name.substr(0, 32).c_str());

    ::moab::ErrorCode rval = m_iface->tag_set_data(m_tag, &entity, 1, &m_tagData);
    return (rval == ::moab::MB_SUCCESS);
  }

  const char* current_name() const { return m_tagData; }
};

class QueryIntTag
{
  ::moab::Interface* m_iface;
  ::moab::TagInfo* m_tag;
  std::string m_tag_name;
  int m_value;
  void const* tag_v_ptr;

public:
  QueryIntTag(const char* name, int value, ::moab::Interface* iface)
  {
    m_iface = iface;
    m_tag_name = std::string(name);
    m_value = value;

    //populate our tag
    ::moab::Tag moab_tag;
    m_iface->tag_get_handle(m_tag_name.c_str(), 1, ::moab::MB_TYPE_INTEGER, moab_tag);

    m_tag = moab_tag;
    //get the memory location of m_value
    this->tag_v_ptr = &m_value;
  }

  ::moab::TagInfo* moabTag() { return m_tag; }

  ::moab::TagInfo* const* moabTagPtr() { return &m_tag; }
  const void* const* moabTagValuePtr() const { return &this->tag_v_ptr; }

  int size() const { return 1; }
  int value() const { return m_value; }
};

class QueryMaterialTag : public QueryIntTag
{
public:
  QueryMaterialTag(int v, ::moab::Interface* iface)
    : QueryIntTag("MATERIAL_SET", v, iface)
  {
  }
};

class QueryDirichletTag : public QueryIntTag
{
public:
  QueryDirichletTag(int v, ::moab::Interface* iface)
    : QueryIntTag("DIRICHLET_SET", v, iface)
  {
  }
};

class QueryNeumannTag : public QueryIntTag
{
public:
  QueryNeumannTag(int v, ::moab::Interface* iface)
    : QueryIntTag("NEUMANN_SET", v, iface)
  {
  }
};

class QueryDimTag : public QueryIntTag
{
public:
  QueryDimTag(int v, ::moab::Interface* iface)
    : QueryIntTag("GEOM_DIMENSION", v, iface)
  {
  }
};

/// A base class for queries on tags that store opaque data values.
template<int Sz>
class QueryOpaqueTag
{
  ::moab::Interface* m_iface;
  ::moab::TagInfo* m_tag;
  std::string m_tag_name;
  char m_value[Sz];
  void const* tag_v_ptr;

public:
  QueryOpaqueTag(const char* name, const char* value, ::moab::Interface* iface)
  {
    m_iface = iface;
    m_tag_name = std::string(name);
    memcpy(m_value, value, Sz);

    //populate our tag
    ::moab::Tag moab_tag;
    m_iface->tag_get_handle(
      m_tag_name.c_str(),
      Sz,
      ::moab::MB_TYPE_OPAQUE,
      moab_tag,
      ::moab::MB_TAG_BYTES | ::moab::MB_TAG_CREAT | ::moab::MB_TAG_SPARSE);

    m_tag = moab_tag;
    //get the memory location of m_value
    this->tag_v_ptr = &m_value;
  }

  ::moab::TagInfo* moabTag() { return m_tag; }

  ::moab::TagInfo* const* moabTagPtr() { return &m_tag; }
  const void* const* moabTagValuePtr() const { return &this->tag_v_ptr; }

  int size() const { return Sz; }
  const char* value() const { return m_value; }
};

class QueryIdTag : public QueryOpaqueTag<smtk::common::UUID::SIZE>
{
public:
  QueryIdTag(::moab::Interface* iface)
    : QueryOpaqueTag("ID", reinterpret_cast<const char*>(smtk::common::UUID::null().begin()), iface)
  {
  }
  QueryIdTag(const smtk::common::UUID& v, ::moab::Interface* iface)
    : QueryOpaqueTag("ID", reinterpret_cast<const char*>(v.begin()), iface)
  {
  }
  smtk::common::UUID uuid() const
  {
    return smtk::common::UUID(
      reinterpret_cast<const unsigned char*>(this->value()),
      reinterpret_cast<const unsigned char*>(this->value()) + smtk::common::UUID::SIZE);
  }
};

class QueryEntRefTag : public QueryOpaqueTag<smtk::common::UUID::SIZE>
{
public:
  QueryEntRefTag(::moab::Interface* iface)
    : QueryOpaqueTag(
        "ENT_REF",
        reinterpret_cast<const char*>(smtk::common::UUID::null().begin()),
        iface)
  {
  }
  QueryEntRefTag(const smtk::common::UUID& v, ::moab::Interface* iface)
    : QueryOpaqueTag("ENT_REF", reinterpret_cast<const char*>(v.begin()), iface)
  {
  }
  smtk::common::UUID uuid() const
  {
    return smtk::common::UUID(
      reinterpret_cast<const unsigned char*>(this->value()),
      reinterpret_cast<const unsigned char*>(this->value()) + smtk::common::UUID::SIZE);
  }
};

class QueryRootModelEntTag : public QueryOpaqueTag<smtk::common::UUID::SIZE>
{
public:
  QueryRootModelEntTag(::moab::Interface* iface)
    : QueryOpaqueTag(
        "ROOT_MODEL_ENT",
        reinterpret_cast<const char*>(smtk::common::UUID::null().begin()),
        iface)
  {
  }
  QueryRootModelEntTag(const smtk::common::UUID& v, ::moab::Interface* iface)
    : QueryOpaqueTag("ROOT_MODEL_ENT", reinterpret_cast<const char*>(v.begin()), iface)
  {
  }
  smtk::common::UUID uuid() const
  {
    return smtk::common::UUID(
      reinterpret_cast<const unsigned char*>(this->value()),
      reinterpret_cast<const unsigned char*>(this->value()) + smtk::common::UUID::SIZE);
  }
};

class QueryBitTag
{
  ::moab::Interface* m_iface;
  ::moab::TagInfo* m_tag;
  std::string m_tag_name;

public:
  QueryBitTag(const char* name, ::moab::Interface* iface)
  {
    m_iface = iface;
    m_tag_name = std::string(name);

    //populate our tag
    ::moab::Tag moab_tag;
    m_iface->tag_get_handle(
      m_tag_name.c_str(),
      1,
      ::moab::MB_TYPE_BIT,
      moab_tag,
      ::moab::MB_TAG_BYTES | ::moab::MB_TAG_CREAT | ::moab::MB_TAG_SPARSE);

    m_tag = moab_tag;
  }

  ::moab::TagInfo* moabTag() { return m_tag; }

  ::moab::TagInfo* const* moabTagPtr() { return &m_tag; }

  int size() const { return 1; }
};

class QueryCellFieldTag : public QueryBitTag
{
public:
  QueryCellFieldTag(const char* name, ::moab::Interface* iface)
    : QueryBitTag(std::string(std::string("c_") + std::string(name)).c_str(), iface)
  {
  }
};

class QueryPointFieldTag : public QueryBitTag
{
public:
  QueryPointFieldTag(const char* name, ::moab::Interface* iface)
    : QueryBitTag(std::string(std::string("p_") + std::string(name)).c_str(), iface)
  {
  }
};

class QueryFieldTag
{
  ::moab::Interface* m_iface;
  ::moab::TagInfo* m_tag;
  ::moab::ErrorCode m_state;
  std::string m_tag_name;
  int m_size;
  ::moab::DataType m_type;

public:
  QueryFieldTag(const char* name, int size, ::moab::DataType type, ::moab::Interface* iface)
  {
    m_iface = iface;
    m_tag_name = std::string(name) + std::string("_");
    m_size = size;
    m_type = type;

    //populate our tag
    ::moab::Tag moab_tag;
    m_state = m_iface->tag_get_handle(
      m_tag_name.c_str(), m_size, m_type, moab_tag, ::moab::MB_TAG_CREAT | ::moab::MB_TAG_DENSE);
    m_tag = moab_tag;
  }

  ::moab::TagInfo* moabTag() { return m_tag; }

  ::moab::TagInfo* const* moabTagPtr() { return &m_tag; }

  ::moab::ErrorCode state() { return m_state; }

  int size() const { return m_size; }

  ::moab::DataType type() const { return m_type; }
};

} // namespace tag
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif

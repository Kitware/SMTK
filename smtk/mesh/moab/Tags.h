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

#include "smtk/mesh/Handle.h"
#include "smtk/mesh/moab/Interface.h"

namespace smtk {
namespace mesh {
namespace moab {
namespace tag  {

//these aren't exported as they are private class that only
//smtk::mesh should call ( currently )

class QueryNameTag
{
  smtk::mesh::moab::Interface* m_iface;
  ::moab::Tag m_tag;
  char m_tagData[32]; //same length as NAME_TAG_SIZE
public:
  QueryNameTag(const smtk::mesh::moab::InterfacePtr& iface);

  bool fetch_name(const smtk::mesh::Handle& entity);

  const char* current_name( ) const { return this->m_tagData; }
};

class QueryDimTag
{
  smtk::mesh::moab::Interface* m_iface;
  ::moab::Tag m_tag;
  int m_dim;
public:
  QueryDimTag(int dimension, const smtk::mesh::moab::InterfacePtr& iface);

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

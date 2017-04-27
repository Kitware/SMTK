//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/MeshListPhrase.h"
#include "smtk/model/EntityPhrase.h"

namespace smtk
{
namespace model
{

MeshListPhrase::MeshListPhrase()
{
}

/**\brief Initialize an mesh list with an iterable container of meshes.
  *
  */
MeshListPhrase::Ptr MeshListPhrase::setup(
  const std::vector<smtk::mesh::MeshSet>& meshes, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(MESH_LIST, parnt);
  for (std::vector<smtk::mesh::MeshSet>::const_iterator it = meshes.begin(); it != meshes.end();
       ++it)
  {
    this->m_meshes.push_back(*it);
  }
  return shared_from_this();
}

MeshListPhrase::Ptr MeshListPhrase::setup(
  const std::vector<smtk::mesh::CollectionPtr>& collections, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(MESH_LIST, parnt);
  for (std::vector<smtk::mesh::CollectionPtr>::const_iterator it = collections.begin();
       it != collections.end(); ++it)
  {
    this->m_collections.push_back(*it);
  }
  return shared_from_this();
}

/// Show the mesh name (or a default name) in the title
std::string MeshListPhrase::title()
{
  std::ostringstream message;
  std::size_t sz =
    this->m_collections.size() > 0 ? this->m_collections.size() : this->m_meshes.size();
  message << sz << " ";

  std::string strDesc = this->m_collections.size() > 0 ? "collections" : "meshsets";

  this->buildSubphrases();
  return message.str();
}

/// Show the entity type in the subtitle
std::string MeshListPhrase::subtitle()
{
  return std::string();
}

/// The list of mehses to be presented.
std::vector<smtk::mesh::MeshSet> MeshListPhrase::relatedMeshes() const
{
  return this->m_meshes;
}
std::vector<smtk::mesh::CollectionPtr> MeshListPhrase::relatedCollections() const
{
  return this->m_collections;
}

} // model namespace
} // smtk namespace

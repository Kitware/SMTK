//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/MeshPhrase.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"

#include <sstream>

namespace smtk {
  namespace model {

MeshPhrase::MeshPhrase()
{
  this->m_mutability = 3; // both color and title are mutable.
}

/**\brief Prepare an MeshPhrase for display a meshset.
  */
MeshPhrase::Ptr MeshPhrase::setup(const smtk::mesh::MeshSet& mesh, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(MESH_SUMMARY, parnt);
  this->m_relatedMesh = mesh;
  this->m_mutability = 3; // both color and title are mutable by default.
  return this->shared_from_this();
}

/**\brief Prepare an MeshPhrase for display a mesh collection.
  */
MeshPhrase::Ptr MeshPhrase::setup(
  const smtk::mesh::CollectionPtr& meshcollection, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(MESH_SUMMARY, parnt);
  this->m_relatedCollection = meshcollection;
  this->m_mutability = 3; // both color and title are mutable by default.
  return this->shared_from_this();
}

bool MeshPhrase::isCollection() const
{
  return this->m_relatedCollection && this->m_relatedCollection->isValid();
}

/// Show the meshset name (or a default name) in the title
std::string MeshPhrase::title()
{
  std::string strText = "no mesh";
  if(this->isCollection())
    {
    //strText = "Meshes";
    strText = this->m_relatedCollection->name().empty() ? "Meshes" : this->m_relatedCollection->name();
    }
  else if(!this->m_relatedMesh.is_empty())
    {
    // trying to use associatied model entity name
    smtk::model::EntityRefArray relatedEnts = this->m_relatedMesh.modelEntities();
    if(relatedEnts.size() == 1)
      {
      strText = relatedEnts[0].name();
      }
    else if(relatedEnts.size() > 1)
      {
      // assuming we have same dimensions in the set
      if(!this->m_relatedMesh.subset(smtk::mesh::Dims3).is_empty())
        {
        strText = "Volumes";
        }
      else if(!this->m_relatedMesh.subset(smtk::mesh::Dims2).is_empty())
        {
        strText = "Faces";
        }
      else if(!this->m_relatedMesh.subset(smtk::mesh::Dims1).is_empty())
        {
        strText = "Edges";
        }
      else
        {
        strText = "unknown meshes";
        }
      }
    }
  return strText;
}

/// True when the meshset is valid and marked as mutable (the default, setMutability(0x1)).
bool MeshPhrase::isTitleMutable() const
{
  return false;//(this->m_mutability & 0x1) && !this->m_relatedMesh.is_empty();
}

bool MeshPhrase::setTitle(const std::string& newTitle)
{
  /*
  // The title is the name, so set the name as long as we're allowed.
  if (this->isTitleMutable() && this->m_relatedMesh.name() != newTitle)
    {
    if (!newTitle.empty())
      this->m_relatedMesh.setName(newTitle);
    else
      {
      this->m_relatedMesh.removeStringProperty("name");
      // Don't let name be a blank... assign a default.
      this->m_relatedMesh.manager()->assignDefaultName(
        this->m_relatedMesh.entity());
      }
    return true;
    }
  */
  return false;
}

/// Return the meshset for additional context the UI might wish to present.
smtk::mesh::MeshSet MeshPhrase::relatedMesh() const
{
  return this->m_relatedMesh;
}

smtk::mesh::CollectionPtr MeshPhrase::relatedMeshCollection() const
{
  return this->m_relatedCollection;
}

/// Return a color associated with the related meshset.
FloatList MeshPhrase::relatedColor() const
{
  return FloatList(4, 1.);//this->m_relatedMesh.color();
}

/// True when the entity is valid and marked as mutable (the default, setMutability(0x4)).
bool MeshPhrase::isRelatedColorMutable() const
{
  return false; //(this->m_mutability & 0x4) && this->m_relatedMesh.isValid();
}

bool MeshPhrase::setRelatedColor(const FloatList& rgba)
{
  /*
  if (this->isRelatedColorMutable())
    {
    bool colorValid = rgba.size() == 4;
    for (int i = 0; colorValid && i < 4; ++i)
      colorValid &= (rgba[i] >= 0. && rgba[i] <= 1.);
    if (colorValid)
      {
      this->m_relatedMesh.setColor(rgba);
      return true;
      }
    }
    */
  return false;
}

// Set bit vector indicating mutability; title (0x1), subtitle (0x2), color (0x4).
void MeshPhrase::setMutability(int whatsMutable)
{
  this->m_mutability = whatsMutable;
}

  } // model namespace
} // smtk namespace

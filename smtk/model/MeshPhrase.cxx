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
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/ShellEntity.h"
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

/**\brief Update the phrase with a new mesh collection.
  * NOTE: This is not updating subphrases, just the related mesh
  */
void MeshPhrase::updateMesh(
  const smtk::mesh::CollectionPtr& meshcollection)
{
  this->m_relatedCollection = meshcollection;
  // update child-phrases' meshes if they are already built, but not triggering
  // rebuild of child phrases' subphrases, which should be handled from ui since they
  // are linked with qtEntityItemModel of the tree view.
  if(this->areSubphrasesBuilt())
    {
    smtk::model::DescriptivePhrases& meshsubs(this->subphrases());
    for (smtk::model::DescriptivePhrases::iterator it = meshsubs.begin();
      it != meshsubs.end(); ++it)
      {
      MeshPhrasePtr mphrase = smtk::dynamic_pointer_cast<MeshPhrase>(*it);
      if(!mphrase)
        continue;
      smtk::mesh::MeshSet relMesh = mphrase->relatedMesh();
      if(relMesh.is_empty())
        continue;
       // assuming we have same dimensions in the set
      if(!relMesh.subset(smtk::mesh::Dims0).is_empty())
        {
        mphrase->updateMesh(meshcollection->meshes(smtk::mesh::Dims0));
        }
      else if(!relMesh.subset(smtk::mesh::Dims1).is_empty())
        {
        mphrase->updateMesh(meshcollection->meshes(smtk::mesh::Dims1));
        }
      else if(!relMesh.subset(smtk::mesh::Dims2).is_empty())
        {
        mphrase->updateMesh(meshcollection->meshes(smtk::mesh::Dims2));
        }
      else if(!relMesh.subset(smtk::mesh::Dims3).is_empty())
        {
        mphrase->updateMesh(meshcollection->meshes(smtk::mesh::Dims3));
        }
      }
    }
}

/**\brief Update the phrase with a new meshset.
  * NOTE: This is not updating subphrases, just the related mesh
  */
void MeshPhrase::updateMesh(
  const smtk::mesh::MeshSet& meshset)
{
  this->m_relatedMesh = meshset;
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
    smtk::mesh::CollectionPtr c = this->m_relatedMesh.collection();
    bool hasValidNameProp = false;
    if(c && c->hasStringProperty(this->m_relatedMesh, "name"))
      {
      smtk::model::StringList const& nprop(c->stringProperty(this->m_relatedMesh, "name"));
      if (!nprop.empty() && !nprop[0].empty())
        {
        strText = nprop[0];
        hasValidNameProp = true;
        }
      }

    if(!hasValidNameProp)
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
        else if(!this->m_relatedMesh.subset(smtk::mesh::Dims0).is_empty())
          {
          strText = "Vertices";
          }
        else
          {
          strText = "unknown meshes";
          }
        }
      }
    }
  return strText;
}

/// True when the meshset is valid and marked as mutable (the default, setMutability(0x1)).
bool MeshPhrase::isTitleMutable() const
{
  return (this->m_mutability & 0x1) && 
   (!this->m_relatedMesh.is_empty() || this->m_relatedCollection->isValid());
}

bool MeshPhrase::setTitle(const std::string& newTitle)
{
  // The title is the name, so set the name as long as we're allowed.
  if (this->isTitleMutable() && newTitle != this->title())
    {
    if(!this->m_relatedMesh.is_empty())
      {
      smtk::mesh::CollectionPtr c = this->m_relatedMesh.collection();
      if(c->isValid())
        {
        c->setStringProperty(this->m_relatedMesh, "name", newTitle);
        return true;
        }
      }
    else if(this->isCollection())
      {
      this->m_relatedCollection->name(newTitle);
      return true;
      }
    }

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
  smtk::mesh::CollectionPtr c;
  smtk::mesh::MeshSet meshkey;
  if(!this->m_relatedMesh.is_empty())
    {
    meshkey = this->m_relatedMesh;
    c = meshkey.collection();
    }
  else
    {
    c = this->relatedMeshCollection();
    meshkey = c->meshes();
    }

  if(c && !meshkey.is_empty())
    {
    FloatList result = c->floatProperty(meshkey, "color");
    int ncomp = static_cast<int>(result.size());
    if (ncomp < 4)
      {
      result.resize(4);
      for (int i = ncomp; i < 3; ++i)
        result[i] = 0.;
      switch (ncomp)
        {
      default:
      case 0: // Assuming color not defined; mark alpha invalid.
        result[3] = -1.;
        break;
      case 1:
      case 3: // Assume RGB or greyscale; default alpha = 1.
        result[3] = 1.;
        break;
      case 2: // Assume greyscale+alpha; remap alpha to result[4]
        result[3] = (result[1] >= 0. && result[1] <= 0. ? result[1] : 1.);
        break;
        }
      }
    return result;
    }

  return FloatList(4, 0.0);
}

/// True when the entity is valid and marked as mutable (the default, setMutability(0x4)).
bool MeshPhrase::isRelatedColorMutable() const
{
  return (this->m_mutability & 0x4) &&
   (!this->m_relatedMesh.is_empty() || this->m_relatedCollection->isValid());
}

bool MeshPhrase::setRelatedColor(const FloatList& rgba)
{
  if (this->isRelatedColorMutable())
    {
    bool colorValid = rgba.size() == 4;
    for (int i = 0; colorValid && i < 4; ++i)
      colorValid &= (rgba[i] >= 0. && rgba[i] <= 1.);
    if (colorValid)
      {
      smtk::mesh::CollectionPtr c;
      smtk::mesh::MeshSet meshkey;
      if(!this->m_relatedMesh.is_empty())
        {
        meshkey = this->m_relatedMesh;
        c = meshkey.collection();
        }
      else
        {
        c = this->relatedMeshCollection();
        meshkey = c->meshes();
        }

      if(c && !meshkey.is_empty())
        {
        c->setFloatProperty(meshkey, "color", rgba);
        return true;
        }
      }
    }
  return false;
}

// Set bit vector indicating mutability; title (0x1), subtitle (0x2), color (0x4).
void MeshPhrase::setMutability(int whatsMutable)
{
  this->m_mutability = whatsMutable;
}

  } // model namespace
} // smtk namespace

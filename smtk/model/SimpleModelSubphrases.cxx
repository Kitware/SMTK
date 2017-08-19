//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SimpleModelSubphrases.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

#include <algorithm>

#include <ctype.h>  // for isdigit
#include <stdlib.h> // for atof

namespace smtk
{
namespace model
{


SimpleModelSubphrases::SimpleModelSubphrases()
  : m_abridgeUses(true)
{
}

DescriptivePhrases SimpleModelSubphrases::subphrases(DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  bool shouldSort(true);
  switch (src->phraseType())
  {
    case ENTITY_SUMMARY:
      this->childrenOfEntity(dynamic_pointer_cast<EntityPhrase>(src), result);
      shouldSort = false;
      break;
    case ENTITY_LIST:
      this->childrenOfEntityList(dynamic_pointer_cast<EntityListPhrase>(src), result);
      break;
    case FLOAT_PROPERTY_LIST:
    case STRING_PROPERTY_LIST:
    case INTEGER_PROPERTY_LIST:
      this->childrenOfPropertyList(dynamic_pointer_cast<PropertyListPhrase>(src), result);
    case MESH_SUMMARY:
      this->childrenOfMesh(dynamic_pointer_cast<MeshPhrase>(src), result);
      break;
    case MESH_LIST:
      this->childrenOfMeshList(dynamic_pointer_cast<MeshListPhrase>(src), result);
      break;
    default:
      break;
  }

  // Now sort the list
  if (shouldSort)
  {
    std::sort(result.begin(), result.end(), DescriptivePhrase::compareByModelInfo);
  }

  return result;
}

bool SimpleModelSubphrases::shouldOmitProperty(
  DescriptivePhrase::Ptr parent, PropertyType ptype, const std::string& pname) const
{
  if (ptype == STRING_PROPERTY)
  {
    if (pname == "name")
      return true;
  }

  if (ptype == FLOAT_PROPERTY)
  {
    if (pname == "color")
      return true;
  }

  if (ptype == INTEGER_PROPERTY)
  {
    if (pname == "visible")
      return true;
    else if (pname == "block_index")
      return true;
    else if (pname == "visibility")
      return true;
    else if (pname == "cmb id")
      return true;
    else if (pname == "membership mask")
      return true;
    else if (pname == "embedding dimension")
      return true;
    else if (pname == SMTK_MESH_GEN_PROP)
      return true;
    else if (parent)
    {
      if (parent->relatedEntity().isModel())
      {
        if (pname.find("_counters") != std::string::npos)
          return true;
        else if (pname.find(SMTK_GEOM_STYLE_PROP) != std::string::npos)
          return true;
      }
      else if (parent->relatedEntity().isCellEntity())
      {
        if (pname.find(SMTK_TESS_GEN_PROP) != std::string::npos)
          return true;
      }
    }
  }
  return false;
}

void SimpleModelSubphrases::setAbridgeUses(bool doAbridge)
{
  this->m_abridgeUses = doAbridge;
}

bool SimpleModelSubphrases::abridgeUses() const
{
  return this->m_abridgeUses;
}

void SimpleModelSubphrases::childrenOfEntity(EntityPhrase::Ptr phr, DescriptivePhrases& result)
{
  // I. Determine dimension of parent.
  //    We will avoid reporting sub-entities if this entity has a
  //    dimension higher than its parent.
  if (!this->m_abridgeUses)
  {
    int dimBits = 0;
    for (DescriptivePhrasePtr pphr = phr->parent(); pphr; pphr = pphr->parent())
    {
      EntityRef c(pphr->relatedEntity());
      if (c.isValid() && c.dimensionBits() > 0)
      {
        dimBits = c.dimensionBits();
        break;
      }
    }
    if (dimBits > 0 && phr->relatedEntity().dimensionBits() > 0 &&
      ((dimBits > phr->relatedEntity().dimensionBits() &&
         !(dimBits & phr->relatedEntity().dimensionBits())) ||
        phr->relatedEntity().isModel()))
    { // Do not report higher-dimensional relation
      return;
    }
  }
  // II. Add arrangement information
  // This is dependent on both the entity type and the ArrangementKind
  // so we cast to different entityref types and use their accessors to
  // obtain lists of related entities.
  EntityRef ent(phr->relatedEntity());
  {
    UseEntity uent = ent.as<UseEntity>();
    CellEntity cent = ent.as<CellEntity>();
    ShellEntity sent = ent.as<ShellEntity>();
    Group gent = ent.as<Group>();
    Model ment = ent.as<Model>();
    Instance ient = ent.as<Instance>();
    SessionRef sess = ent.as<SessionRef>();
    if (uent.isValid())
    {
      this->cellOfUse(phr, uent, result);
      this->boundingShellsOfUse(phr, uent, result);
      this->toplevelShellsOfUse(phr, uent, result);
    }
    else if (cent.isValid())
    {
      if (!this->m_abridgeUses)
        this->usesOfCell(phr, cent, result);
      else
        this->boundingCellsOfCell(phr, cent, result);
      this->inclusionsOfCell(phr, cent, result);
    }
    else if (sent.isValid())
    {
      this->usesOfShell(phr, sent, result);
    }
    else if (gent.isValid())
    {
      this->membersOfGroup(phr, gent, result);
    }
    // only expand active model
    else if (ment.isValid() && (ment.entity() == this->activeModel().entity()))
    {
      this->freeSubmodelsOfModel(phr, ment, result);
      this->freeGroupsInModel(phr, ment, result);
      this->freeCellsOfModel(phr, ment, result);
      this->freeAuxiliaryGeometriesOfModel(phr, ment, result);
      this->meshesOfModel(phr, ment, result);
    }
    else if (ient.isValid())
    {
      this->prototypeOfInstance(phr, ient, result);
    }
    else if (sess.isValid())
    {
      this->modelsOfSession(phr, sess, result);
    }
  }
  // Things common to all entities
  this->instancesOfEntity(phr, ent, result);
  // III. Add attribute information
  this->attributesOfEntity(phr, ent, result);
  // IV. Add property information
  this->propertiesOfEntity(phr, ent, result);
}

void SimpleModelSubphrases::childrenOfEntityList(
  EntityListPhrase::Ptr elist, DescriptivePhrases& result)
{
  this->entitiesOfEntityList(elist, elist->relatedEntities(), result);
}

void SimpleModelSubphrases::childrenOfPropertyList(
  PropertyListPhrase::Ptr plist, DescriptivePhrases& result)
{
  this->propertiesOfPropertyList(plist, plist->relatedPropertyType(), result);
}

void SimpleModelSubphrases::childrenOfMesh(MeshPhrase::Ptr meshphr, DescriptivePhrases& result)
{
  if (meshphr)
  {
    this->meshsetsOfMesh(meshphr, result);
  }
}

void SimpleModelSubphrases::childrenOfMeshList(
  MeshListPhrase::Ptr meshlist, DescriptivePhrases& result)
{
  this->meshesOfMeshList(meshlist, result);
}

} // namespace model
} // namespace smtk

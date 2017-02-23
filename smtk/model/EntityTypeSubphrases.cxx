//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EntityTypeSubphrases.h"

#include "smtk/model/SessionRef.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

#include <algorithm>

#include <ctype.h> // for isdigit
#include <stdlib.h> // for atof

namespace smtk {
  namespace model {

static bool SpecialEntityNameSort(const DescriptivePhrasePtr& a, const DescriptivePhrasePtr& b)
{
  static const int sortOrder[] = {
     1, // ENTITY_LIST
     0, // ENTITY_SUMMARY
     2, // ARRANGEMENT_LIST
     3, // ATTRIBUTE_LIST
     4, // MODEL_INCLUDES_ENTITY
     5, // MODEL_EMBEDDED_IN_MODEL
     6, // CELL_INCLUDES_CELL
     7, // CELL_EMBEDDED_IN_CELL
     8, // CELL_HAS_SHELL
     9, // CELL_HAS_USE
    10, // SHELL_HAS_CELL
    11, // SHELL_HAS_USE
    12, // USE_HAS_CELL
    13, // USE_HAS_SHELL
    15, // FLOAT_PROPERTY_LIST
    16, // STRING_PROPERTY_LIST
    14, // INTEGER_PROPERTY_LIST
    18, // ENTITY_HAS_FLOAT_PROPERTY
    19, // ENTITY_HAS_STRING_PROPERTY
    17, // ENTITY_HAS_INTEGER_PROPERTY
    20, // ENTITY_HAS_ATTRIBUTE
    22, // FLOAT_PROPERTY_VALUE
    23, // STRING_PROPERTY_VALUE
    21, // INTEGER_PROPERTY_VALUE
    24, // ENTITY_HAS_SUBPHRASES
    25  // INVALID_DESCRIPTION
  };

  // I. Sort by phrase type.
  DescriptivePhraseType pta = a->phraseType();
  DescriptivePhraseType ptb = b->phraseType();
  if (pta != ptb)
    {
      return sortOrder[pta] < sortOrder[ptb];
    }

  // II. Sort by entity type/dimension
  // II.a. Entity type
  BitFlags eta = a->relatedEntity().entityFlags() & ENTITY_MASK;
  BitFlags etb = b->relatedEntity().entityFlags() & ENTITY_MASK;
  if (eta != etb)
    {
    switch (eta)
      {
    case CELL_ENTITY:       // 0x0100
      return etb == MODEL_ENTITY ? false : true;
    case USE_ENTITY:        // 0x0200
      return etb == MODEL_ENTITY || etb < USE_ENTITY ? false : true;
    case SHELL_ENTITY:      // 0x0400
      return etb == MODEL_ENTITY || etb < SHELL_ENTITY ? false : true;
    case GROUP_ENTITY:      // 0x0800
      return etb == MODEL_ENTITY || etb < SHELL_ENTITY ? false : true;
    case MODEL_ENTITY:      // 0x1000
      return true;
    case INSTANCE_ENTITY:   // 0x2000
      return false;
    default:
      return eta < etb ? true : false;
      }
    }

  // II.b. Dimension
  eta = a->relatedEntity().entityFlags() & ANY_DIMENSION;
  etb = b->relatedEntity().entityFlags() & ANY_DIMENSION;
  if (eta != etb)
    return eta < etb;

  // III. Sort by title, with care taken when differences are numeric values.
  std::string ta(a->title());
  std::string tb(b->title());
  if (ta.empty())
    return true;
  if (tb.empty())
    return false;
  std::string::size_type minlen = ta.size() < tb.size() ? ta.size() : tb.size();
  std::string::size_type i;
  for (i = 0; i < minlen; ++i)
    if (ta[i] != tb[i])
      break; // Stop at the first difference between ta and tb.

  // Shorter strings are less than longer versions with the same start:
  if (i == minlen)
    return ta.size() < tb.size() ? true : false;

  // Both ta & tb have some character present and different.
  bool da = isdigit(ta[i]) ? true : false;
  bool db = isdigit(tb[i]) ? true : false;
  if ( da && !db) return true; // digits come before other things
  if (!da &&  db) return false; // non-digits come after digits
  if (!da && !db) return ta[i] < tb[i];
  // Now, both ta and tb differ with some numeric value.
  // Convert to a number and compare the numbers.
  double na = atof(ta.substr(i).c_str());
  double nb = atof(tb.substr(i).c_str());
  return na < nb;
}

EntityTypeSubphrases::EntityTypeSubphrases()
{
  m_directlimit = 100;
}

DescriptivePhrases EntityTypeSubphrases::subphrases(
  DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  bool shouldSort(true);
  switch (src->phraseType())
    {
  case ENTITY_SUMMARY:
    this->childrenOfEntity(
      dynamic_pointer_cast<EntityPhrase>(src), result);
    shouldSort = false; // use the order when calling internal_createEntityList()
    break;
  case ENTITY_LIST:
    this->childrenOfEntityList(
      dynamic_pointer_cast<EntityListPhrase>(src), result);
    break;
  case FLOAT_PROPERTY_LIST:
  case STRING_PROPERTY_LIST:
  case INTEGER_PROPERTY_LIST:
    this->childrenOfPropertyList(
      dynamic_pointer_cast<PropertyListPhrase>(src), result);
  case MESH_SUMMARY:
    this->childrenOfMesh(
      dynamic_pointer_cast<MeshPhrase>(src), result);
    break;
  case MESH_LIST:
    this->childrenOfMeshList(
      dynamic_pointer_cast<MeshListPhrase>(src), result);
    break;
  default:
    break;
    }

  // Now sort the list
  if (shouldSort)
    {
      std::sort(result.begin(), result.end(), SpecialEntityNameSort);
    }

  return result;
}

bool EntityTypeSubphrases::shouldOmitProperty(
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

/// Recursively find all the cell entities
inline void internal_findEntities(
  const EntityRef& root,
  EntityRefs& vols, EntityRefs& faces, EntityRefs& edges,
  EntityRefs& verts, EntityRefs& aux,
  std::set<smtk::model::EntityRef>& touched)
{
  EntityRefArray children =
    (root.isModel() ?
     root.as<Model>().cellsAs<EntityRefArray>() :
     (root.isCellEntity() ?
      root.as<CellEntity>().boundingCellsAs<EntityRefArray>() :
      (root.isGroup() ?
       root.as<Group>().members<EntityRefArray>() :
       EntityRefArray())));
  if (root.isModel())
    {
    // Make sure groups are handled last to avoid unexpected "parents" in entityrefMap.
    EntityRefArray tmp;
    tmp = root.as<Model>().submodelsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
    tmp = root.as<Model>().groupsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
    AuxiliaryGeometries freeAuxGeom = root.as<Model>().auxiliaryGeometry();
    children.insert(children.end(), freeAuxGeom.begin(), freeAuxGeom.end());
    }
  for (EntityRefArray::const_iterator it = children.begin(); it != children.end(); ++it)
    {
    if (touched.find(*it) == touched.end())
      {
      touched.insert(*it);
      if (it->isVolume())
        {
        vols.insert(*it);
        }
      else if(it->isFace())
        {
        faces.insert(*it);
        }
      else if(it->isEdge())
        {
        edges.insert(*it);
        }
      else if(it->isVertex())
        {
        verts.insert(*it);
        }
      else if (it->isAuxiliaryGeometry())
        {
        aux.insert(*it);
        }

      internal_findEntities(*it, vols, faces, edges, verts, aux, touched);
      }
    }
}

inline void internal_createEntityList(
  const EntityRefs& ents, EntityPhrase::Ptr phr,
  DescriptivePhrases& result)
{
  if(ents.size() > 0)
    result.push_back(
      EntityListPhrase::create()->setup(ents, phr));
}

void EntityTypeSubphrases::childrenOfEntity(
  EntityPhrase::Ptr phr, DescriptivePhrases& result)
{
  // II. Add arrangement information
  // This is dependent on both the entity type and the ArrangementKind
  // so we cast to different entityref types and use their accessors to
  // obtain lists of related entities.
  EntityRef ent(phr->relatedEntity());
    {
//    UseEntity uent = ent.as<UseEntity>();
//    CellEntity cent = ent.as<CellEntity>();
//    ShellEntity sent = ent.as<ShellEntity>();
    Group gent = ent.as<Group>();
    Model ment = ent.as<Model>();
    Instance ient = ent.as<Instance>();
    SessionRef sess = ent.as<SessionRef>();
/*
    if (uent.isValid())
      {
      this->cellOfUse(phr, uent, result);
      this->boundingShellsOfUse(phr, uent, result);
      this->toplevelShellsOfUse(phr, uent, result);
      }
    else if (cent.isValid())
      {
      this->boundingCellsOfCell(phr, cent, result);
      this->inclusionsOfCell(phr, cent, result);
      }
    else if (sent.isValid())
      {
      this->usesOfShell(phr, sent, result);
      }
*/
    if (gent.isValid())
      {
      this->membersOfGroup(phr, gent, result);
      }
    else if (ment.isValid())
      {
      this->freeSubmodelsOfModel(phr, ment, result);
      this->freeGroupsInModel(phr, ment, result);

      EntityRefs vols, faces, edges, verts, aux, touched;
      internal_findEntities(ment, vols, faces, edges, verts, aux, touched);

      internal_createEntityList(vols, phr, result);
      internal_createEntityList(faces, phr, result);
      internal_createEntityList(edges, phr, result);
      internal_createEntityList(verts, phr, result);
      internal_createEntityList(aux, phr, result);

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

void EntityTypeSubphrases::childrenOfEntityList(
  EntityListPhrase::Ptr elist, DescriptivePhrases& result)
{
  this->entitiesOfEntityList(elist, elist->relatedEntities(), result);
}

void EntityTypeSubphrases::childrenOfPropertyList(
  PropertyListPhrase::Ptr plist, DescriptivePhrases& result)
{
  this->propertiesOfPropertyList(plist, plist->relatedPropertyType(), result);
}

void EntityTypeSubphrases::childrenOfMesh(
  MeshPhrase::Ptr meshphr, DescriptivePhrases& result)
{
  this->meshsetsOfMesh(meshphr, result);
}

void EntityTypeSubphrases::childrenOfMeshList(
  MeshListPhrase::Ptr meshlist, DescriptivePhrases& result)
{
  this->meshesOfMeshList(meshlist, result);
}

  } // namespace model
} // namespace smtk

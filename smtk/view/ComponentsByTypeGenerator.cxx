//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ComponentsByTypeSubphrases.h"

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
namespace view
{

ComponentsByTypeSubphrases::ComponentsByTypeSubphrases()
{
  m_directLimit = 100;
}

DescriptivePhrases ComponentsByTypeSubphrases::subphrases(DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  if (!src)
  {
    return result;
  }

  bool shouldSort(true);
  auto rsrc = src->relatedResource();
  auto comp = src->relatedComponent();
  switch (src->phraseType())
  {
    case DescriptivePhraseType::RESOURCE_SUMMARY:
      if (rsrc)
      {
        this->componentsOfResource(src, rsrc, result);
      }
      break;
    case DescriptivePhraseType::RESOURCE_LIST:
    {
      auto rsrcMgr = this->resourceManager();
      if (rsrcMgr)
      {
        this->resourcesOfManager(src, rsrcMgr, result);
      }
    }
    break;
    case DescriptivePhraseType::COMPONENT_SUMMARY:
    case DescriptivePhraseType::COMPONENT_LIST:
    case DescriptivePhraseType::PROPERTY_LIST:
    case DescriptivePhraseType::FLOAT_PROPERTY_VALUE:
    case DescriptivePhraseType::STRING_PROPERTY_VALUE:
    case DescriptivePhraseType::INTEGER_PROPERTY_VALUE:
    case DescriptivePhraseType::LIST:
    case DescriptivePhraseType::INVALID_DESCRIPTION:

    case ENTITY_SUMMARY:
      this->childrenOfEntity(dynamic_pointer_cast<EntityPhrase>(src), result);
      shouldSort = false; // use the order when calling internal_createEntityList()
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

bool ComponentsByTypeSubphrases::shouldOmitProperty(
  DescriptivePhrase::Ptr parent, smtk::resource::PropertyType ptype, const std::string& pname) const
{
  if (ptype == smtk::resource::STRING_PROPERTY)
  {
    if (pname == "name")
      return true;
  }

  if (ptype == smtk::resource::FLOAT_PROPERTY)
  {
    if (pname == "color")
      return true;
  }

  if (ptype == smtk::resource::INTEGER_PROPERTY)
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
inline void internal_findEntities(const EntityRef& root, EntityRefs& vols, EntityRefs& faces,
  EntityRefs& edges, EntityRefs& verts, EntityRefs& aux, std::set<smtk::model::EntityRef>& touched)
{
  EntityRefArray children = (root.isModel()
      ? root.as<Model>().cellsAs<EntityRefArray>()
      : (root.isCellEntity()
            ? root.as<CellEntity>().boundingCellsAs<EntityRefArray>()
            : (root.isGroup() ? root.as<Group>().members<EntityRefArray>() : EntityRefArray())));
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
      else if (it->isFace())
      {
        faces.insert(*it);
      }
      else if (it->isEdge())
      {
        edges.insert(*it);
      }
      else if (it->isVertex())
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
  const EntityRefs& ents, EntityPhrase::Ptr phr, DescriptivePhrases& result)
{
  if (ents.size() > 0)
    result.push_back(EntityListPhrase::create()->setup(ents, phr));
}

void ComponentsByTypeSubphrases::childrenOfModelEntity(
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
    AuxiliaryGeometry aent = ent.as<AuxiliaryGeometry>();
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
    if (aent.isValid())
    {
      this->childrenOfAuxiliaryGeometry(phr, aent, result);
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

      EntityRefs vols, faces, edges, verts, aux, touched;
      // internal_findEntities(ment, vols, faces, edges, verts, aux, touched);
      this->collectEntitiesByType(ment, vols, faces, edges, verts, aux, touched);

      if (!vols.empty())
      {
        result.push_back(PhraseList::create()->setup(vols, phr));
      }
      internal_createEntityList(vols, phr, result);
      internal_createEntityList(faces, phr, result);
      internal_createEntityList(edges, phr, result);
      internal_createEntityList(verts, phr, result);
      internal_createEntityList(aux, phr, result);

      this->meshesOfModel(phr, ment, result);
    }
    /* For now, to prevent infinite recursion because some things
     * are foolishly using the tree view to create a selection
     * (qtModelView ::selectionChanged and ::currentSelectionByMask,
     * I'm lookin' at you!), do not report the prototype of an instance
     * as one of its children.
    else if (ient.isValid())
    {
      this->prototypeOfInstance(phr, ient, result);
    }
     */
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

void ComponentsByTypeSubphrases::childrenOfEntityList(
  EntityListPhrase::Ptr elist, DescriptivePhrases& result)
{
  this->entitiesOfEntityList(elist, elist->relatedEntities(), result);
}

void ComponentsByTypeSubphrases::childrenOfPropertyList(
  PropertyListPhrase::Ptr plist, DescriptivePhrases& result)
{
  this->propertiesOfPropertyList(plist, plist->relatedPropertyType(), result);
}

void ComponentsByTypeSubphrases::childrenOfMesh(MeshPhrase::Ptr meshphr, DescriptivePhrases& result)
{
  this->meshsetsOfMesh(meshphr, result);
}

void ComponentsByTypeSubphrases::childrenOfMeshList(
  MeshListPhrase::Ptr meshlist, DescriptivePhrases& result)
{
  this->meshesOfMeshList(meshlist, result);
}

} // namespace view
} // namespace smtk

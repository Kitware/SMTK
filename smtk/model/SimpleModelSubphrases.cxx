#include "smtk/model/SimpleModelSubphrases.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/InstanceEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

namespace smtk {
  namespace model {

SimpleModelSubphrases::SimpleModelSubphrases()
  : m_passOverUses(true)
{
}

DescriptivePhrases SimpleModelSubphrases::subphrases(
  DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  switch (src->phraseType())
    {
  case ENTITY_SUMMARY:
    this->childrenOfEntity(
      dynamic_pointer_cast<EntityPhrase>(src), result);
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
  default:
    break;
    }
  return result;
}

bool SimpleModelSubphrases::shouldOmitProperty(
  DescriptivePhrase::Ptr parent, PropertyType ptype, const std::string& pname) const
{
  if (ptype == STRING_PROPERTY && pname == "name")
    return true;

  if (ptype == FLOAT_PROPERTY && pname == "color")
    return true;

  if (
    ptype == INTEGER_PROPERTY &&
    parent && parent->relatedEntity().isModelEntity())
    {
    if (pname.find("_counters") != std::string::npos)
      return true;
    }
  return false;
}

void SimpleModelSubphrases::childrenOfEntity(
  EntityPhrase::Ptr phr, DescriptivePhrases& result)
{
  // I. Determine dimension of parent.
  //    We will avoid reporting sub-entities if this entity has a
  //    dimension higher than its parent.
  if (!this->m_passOverUses)
    {
    int dimBits = 0;
    for (DescriptivePhrasePtr pphr = phr->parent(); pphr; pphr = pphr->parent())
      {
      Cursor c(pphr->relatedEntity());
      if (c.isValid() && c.dimensionBits() > 0)
        {
        dimBits = c.dimensionBits();
        break;
        }
      }
    if (
      dimBits > 0 && phr->relatedEntity().dimensionBits() > 0 && (
        (dimBits > phr->relatedEntity().dimensionBits() && !(dimBits & phr->relatedEntity().dimensionBits())) ||
        phr->relatedEntity().isModelEntity()))
      { // Do not report higher-dimensional relation
      return;
      }
    }
  // II. Add arrangement information
  // This is dependent on both the entity type and the ArrangementKind
  // so we cast to different cursor types and use their accessors to
  // obtain lists of related entities.
  Cursor ent(phr->relatedEntity());
    {
    UseEntity uent = ent.as<UseEntity>();
    CellEntity cent = ent.as<CellEntity>();
    ShellEntity sent = ent.as<ShellEntity>();
    GroupEntity gent = ent.as<GroupEntity>();
    ModelEntity ment = ent.as<ModelEntity>();
    InstanceEntity ient = ent.as<InstanceEntity>();
    if (uent.isValid())
      {
      this->cellOfUse(phr, uent, result);
      this->boundingShellsOfUse(phr, uent, result);
      this->toplevelShellsOfUse(phr, uent, result);
      }
    else if (cent.isValid())
      {
      this->toplevelShellsOfCell(phr, cent, result);
      if (!this->m_passOverUses)
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
    else if (ment.isValid())
      {
      this->freeSubmodelsOfModel(phr, ment, result);
      this->freeGroupsInModel(phr, ment, result);
      this->freeCellsOfModel(phr, ment, result);
      }
    else if (ient.isValid())
      {
      this->prototypeOfInstance(phr, ient, result);
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

  } // namespace model
} // namespace smtk

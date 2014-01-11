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

DescriptivePhrases SimpleModelSubphrases::subphrases(
  DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  switch (src->phraseType())
    {
  case ENTITY_SUMMARY:
    this->ChildrenOfEntity(
      dynamic_pointer_cast<EntityPhrase>(src), result);
    break;
  case ENTITY_LIST:
    this->ChildrenOfEntityList(
      dynamic_pointer_cast<EntityListPhrase>(src), result);
    break;
  case FLOAT_PROPERTY_LIST:
  case STRING_PROPERTY_LIST:
  case INTEGER_PROPERTY_LIST:
    this->ChildrenOfPropertyList(
      dynamic_pointer_cast<PropertyListPhrase>(src), result);
  default:
    break;
    }
  return result;
}

SimpleModelSubphrases::SimpleModelSubphrases()
  : m_passOverUses(true)
{
}

void SimpleModelSubphrases::ChildrenOfEntity(
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
      this->CellOfUse(phr, uent, result);
      this->BoundingShellsOfUse(phr, uent, result);
      this->ToplevelShellsOfUse(phr, uent, result);
      }
    else if (cent.isValid())
      {
      this->ToplevelShellsOfCell(phr, cent, result);
      if (!this->m_passOverUses)
        this->UsesOfCell(phr, cent, result);
      else
        this->BoundingCellsOfCell(phr, cent, result);
      this->InclusionsOfCell(phr, cent, result);
      }
    else if (sent.isValid())
      {
      this->UsesOfShell(phr, sent, result);
      }
    else if (gent.isValid())
      {
      this->MembersOfGroup(phr, gent, result);
      }
    else if (ment.isValid())
      {
      this->FreeSubmodelsOfModel(phr, ment, result);
      this->FreeGroupsInModel(phr, ment, result);
      this->FreeCellsOfModel(phr, ment, result);
      }
    else if (ient.isValid())
      {
      this->PrototypeOfInstance(phr, ient, result);
      }
    }
  // Things common to all entities
  this->InstancesOfEntity(phr, ent, result);
  // III. Add attribute information
  this->AttributesOfEntity(phr, ent, result);
  // IV. Add property information
  this->PropertiesOfEntity(phr, ent, result);
}

void SimpleModelSubphrases::ChildrenOfEntityList(
  EntityListPhrase::Ptr elist, DescriptivePhrases& result)
{
  this->EntitiesOfEntityList(elist, elist->relatedEntities(), result);
}

void SimpleModelSubphrases::ChildrenOfPropertyList(
  PropertyListPhrase::Ptr plist, DescriptivePhrases& result)
{
  this->PropertiesOfPropertyList(plist, plist->relatedPropertyType(), result);
}

  } // namespace model
} // namespace smtk

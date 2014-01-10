#include "smtk/model/EntityPhrase.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/InstanceEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

namespace smtk {
  namespace model {

EntityPhrase::EntityPhrase()
{
}

/**\brief Prepare an EntityPhrase for display.
  */
EntityPhrase::Ptr EntityPhrase::setup(const Cursor& entity, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(ENTITY_SUMMARY, parnt);
  this->m_entity = entity;
  return this->shared_from_this();
}

/// Show the entity name (or a default name) in the title
std::string EntityPhrase::title()
{
  return this->m_entity.name();
}

/// Show the entity type in the subtitle
std::string EntityPhrase::subtitle()
{
  return this->m_entity.flagSummary();
}

/// Return the entity for additional context the UI might wish to present.
Cursor EntityPhrase::relatedEntity() const
{
  return this->m_entity;
}

DescriptivePhrases EntityPhrase::PhrasesFromUUIDs(
  smtk::model::StoragePtr storage, const smtk::util::UUIDs& uids)
{
  DescriptivePhrases result;
  smtk::util::UUIDs::const_iterator it;
  for (it = uids.begin(); it != uids.end(); ++it)
    {
    result.push_back(
      EntityPhrase::create()->setup(Cursor(storage, *it)));
    }
  return result;
}

bool EntityPhrase::buildSubphrasesInternal()
{
  // I. Determine dimension of parent.
  //    We will avoid reporting sub-entities if this entity has a
  //    dimension higher than its parent.
  int dimBits = 0;
  for (DescriptivePhrasePtr phr = this->parent(); phr; phr = phr->parent())
    {
    Cursor c(phr->relatedEntity());
    if (c.isValid() && c.dimensionBits() > 0)
      {
      dimBits = c.dimensionBits();
      break;
      }
    }
  if (
    dimBits > 0 && this->relatedEntity().dimensionBits() > 0 && (
      (dimBits > this->relatedEntity().dimensionBits() && !(dimBits & this->relatedEntity().dimensionBits())) ||
      this->relatedEntity().isModelEntity()))
    {
    return true;
    }
  // II. Add arrangement information
  // This is dependent on both the entity type and the ArrangementKind
  // so we cast to different cursor types and use their accessors to
  // obtain lists of related entities.
    {
    UseEntity uent = this->m_entity.as<UseEntity>();
    CellEntity cent = this->m_entity.as<CellEntity>();
    ShellEntity sent = this->m_entity.as<ShellEntity>();
    GroupEntity gent = this->m_entity.as<GroupEntity>();
    ModelEntity ment = this->m_entity.as<ModelEntity>();
    InstanceEntity ient = this->m_entity.as<InstanceEntity>();
    if (uent.isValid())
      {
      CellEntity parentCell = uent.cell();
      if (parentCell.isValid())
        {
        this->m_subphrases.push_back(
          EntityPhrase::create()->setup(parentCell, shared_from_this()));
        }
      ShellEntities boundingShells = uent.boundingShellEntities<ShellEntities>();
      if (!boundingShells.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            boundingShells, shared_from_this()));
        }
      ShellEntities toplevelShells = uent.shellEntities<ShellEntities>();
      if (!toplevelShells.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            toplevelShells, shared_from_this()));
        }
      }
    else if (cent.isValid())
      {
      ShellEntities toplevelShells = cent.shellEntities();
      if (!toplevelShells.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            toplevelShells, shared_from_this()));
        }
      UseEntities cellUses = cent.uses<UseEntities>();
      if (!cellUses.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            cellUses, shared_from_this()));
        }
      Cursors inclusions = cent.inclusions<Cursors>();
      if (!inclusions.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            inclusions, shared_from_this()));
        }
      }
    else if (sent.isValid())
      {
      UseEntities shellUses = sent.uses<UseEntities>();
      if (!shellUses.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            shellUses, shared_from_this()));
        }
      }
    else if (gent.isValid())
      {
      CursorArray members = gent.members<CursorArray>();
      if (!members.empty())
        { // TODO: Sort by entity type, name, etc.?
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            members, shared_from_this()));
        }
      }
    else if (ment.isValid())
      {
      ModelEntities freeSubmodelsInModel = ment.submodels();
      if (!freeSubmodelsInModel.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            freeSubmodelsInModel, shared_from_this()));
        }
      GroupEntities freeGroupsInModel = ment.groups();
      if (!freeGroupsInModel.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            freeGroupsInModel, shared_from_this()));
        }
      CellEntities freeCellsInModel = ment.cells();
      if (!freeCellsInModel.empty())
        {
        this->m_subphrases.push_back(
          EntityListPhrase::create()->setup(
            freeCellsInModel, shared_from_this()));
        }
      }
    else if (ient.isValid())
      {
      Cursor instanceOf = ient.prototype();
      if (instanceOf.isValid())
        {
        this->m_subphrases.push_back(
          EntityPhrase::create()->setup(
            instanceOf, shared_from_this()));
        }
      }
    }
  // Things common to all entities
  InstanceEntities instances = this->m_entity.instances<InstanceEntities>();
  if (!instances.empty())
    {
    this->m_subphrases.push_back(
      EntityListPhrase::create()->setup(
        instances, shared_from_this()));
    }
  // III. Add attribute information
  // TODO.
  // IV. Add property information
  if (this->m_entity.hasStringProperties())
    { // TODO: If m_entity.stringProperties().size() < N, add PropValuePhrases instead of a list.
    PropertyListPhrase::Ptr plist =
      PropertyListPhrase::create()->setup(
        this->m_entity, STRING_PROPERTY, shared_from_this());
    if (!plist->subphrases().empty())
      {
      this->m_subphrases.push_back(plist);
      }
    }
  if (this->m_entity.hasFloatProperties())
    { // TODO: If m_entity.floatProperties().size() < N, add PropValuePhrases instead of a list.
    this->m_subphrases.push_back(
      PropertyListPhrase::create()->setup(
        this->m_entity, FLOAT_PROPERTY, shared_from_this()));
    }
  if (this->m_entity.hasIntegerProperties())
    { // TODO: If m_entity.integerProperties().size() < N, add PropValuePhrases instead of a list.
    this->m_subphrases.push_back(
      PropertyListPhrase::create()->setup(
        this->m_entity, INTEGER_PROPERTY, shared_from_this()));
    }
  return true;
}

  } // model namespace
} // smtk namespace

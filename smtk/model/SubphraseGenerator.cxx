#include "smtk/model/SubphraseGenerator.h"

#include "smtk/model/ModelEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/InstanceEntity.h"

#include "smtk/model/FloatData.h"
#include "smtk/model/StringData.h"
#include "smtk/model/IntegerData.h"

#include "smtk/model/AttributeListPhrase.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/PropertyValuePhrase.h"

namespace smtk {
  namespace model {

void SubphraseGenerator::InstancesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  InstanceEntities instances = ent.instances<InstanceEntities>();
  if (!instances.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        instances, src));
    }
}

void SubphraseGenerator::AttributesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  if (ent.hasAttributes())
    {
    result.push_back(
      AttributeListPhrase::create()->setup(
        ent, ent.attributes(), src));
    }
}

void SubphraseGenerator::PropertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  this->StringPropertiesOfEntity(src, ent, result);
  this->IntegerPropertiesOfEntity(src, ent, result);
  this->FloatPropertiesOfEntity(src, ent, result);
}

void SubphraseGenerator::FloatPropertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  if (ent.hasFloatProperties())
    { // TODO: If m_entity.floatProperties().size() < N, add PropValuePhrases instead of a list.
    result.push_back(
      PropertyListPhrase::create()->setup(
        ent, FLOAT_PROPERTY, src));
    }
}

void SubphraseGenerator::StringPropertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  if (ent.hasStringProperties())
    { // TODO: If m_entity.stringProperties().size() < N, add PropValuePhrases instead of a list.
    // FIXME: filter ent.stringProperties() keys and create either PropertyListPhrase or PropertyValuePhrase(s).
    PropertyListPhrase::Ptr plist =
      PropertyListPhrase::create()->setup(
        ent, STRING_PROPERTY, src);
    if (!plist->subphrases().empty())
      { // FIXME: Bad form to call plist->subphrases()... remove in favor of FIXME above.
      result.push_back(plist);
      }
    }
}

void SubphraseGenerator::IntegerPropertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  if (ent.hasIntegerProperties())
    { // TODO: If m_entity.integerProperties().size() < N, add PropValuePhrases instead of a list.
    result.push_back(
      PropertyListPhrase::create()->setup(
        ent, INTEGER_PROPERTY, src));
    }
}


void SubphraseGenerator::CellOfUse(
  DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result)
{
  CellEntity parentCell = ent.cell();
  if (parentCell.isValid())
    {
    result.push_back(
      EntityPhrase::create()->setup(parentCell, src));
    }
}

void SubphraseGenerator::BoundingShellsOfUse(
  DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result)
{
  ShellEntities boundingShells = ent.boundingShellEntities<ShellEntities>();
  if (!boundingShells.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        boundingShells, src));
    }
}

void SubphraseGenerator::ToplevelShellsOfUse(
  DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result)
{
  ShellEntities toplevelShells = ent.shellEntities<ShellEntities>();
  if (!toplevelShells.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        toplevelShells, src));
    }
}


void SubphraseGenerator::ToplevelShellsOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  ShellEntities toplevelShells = ent.shellEntities();
  if (!toplevelShells.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        toplevelShells, src));
    }
}

void SubphraseGenerator::UsesOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  UseEntities cellUses = ent.uses<UseEntities>();
  if (!cellUses.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        cellUses, src));
    }
}

void SubphraseGenerator::InclusionsOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  Cursors inclusions = ent.inclusions<Cursors>();
  if (!inclusions.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        inclusions, src));
    }
}


void SubphraseGenerator::UsesOfShell(
  DescriptivePhrase::Ptr src, const ShellEntity& ent, DescriptivePhrases& result)
{
  UseEntities shellUses = ent.uses<UseEntities>();
  if (!shellUses.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        shellUses, src));
    }
}


void SubphraseGenerator::MembersOfGroup(
  DescriptivePhrase::Ptr src, const GroupEntity& grp, DescriptivePhrases& result)
{
  CursorArray members = grp.members<CursorArray>();
  if (!members.empty())
    { // TODO: Sort by entity type, name, etc.?
    result.push_back(
      EntityListPhrase::create()->setup(
        members, src));
    }
}

void SubphraseGenerator::FreeSubmodelsOfModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  ModelEntities freeSubmodelsInModel = mod.submodels();
  if (!freeSubmodelsInModel.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        freeSubmodelsInModel, src));
    }
}

void SubphraseGenerator::FreeGroupsInModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  GroupEntities freeGroupsInModel = mod.groups();
  if (!freeGroupsInModel.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        freeGroupsInModel, src));
    }
}

void SubphraseGenerator::FreeCellsOfModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  CellEntities freeCellsInModel = mod.cells();
  if (!freeCellsInModel.empty())
    {
    result.push_back(
      EntityListPhrase::create()->setup(
        freeCellsInModel, src));
    }
}


void SubphraseGenerator::PrototypeOfInstance(
  DescriptivePhrase::Ptr src, const InstanceEntity& ent, DescriptivePhrases& result)
{
  Cursor instanceOf = ent.prototype();
  if (instanceOf.isValid())
    {
    result.push_back(
      EntityPhrase::create()->setup(
        instanceOf, src));
    }
}


void SubphraseGenerator::EntitiesOfEntityList(
  DescriptivePhrase::Ptr src, const CursorArray& ents, DescriptivePhrases& result)
{
}

void SubphraseGenerator::PropertiesOfPropertyList(
   PropertyListPhrase::Ptr src, PropertyType p, DescriptivePhrases& result)
{
  switch (src->relatedPropertyType())
    {
  case FLOAT_PROPERTY:
    if (src->relatedEntity().hasFloatProperties())
      {
      for (
        PropertyNameWithConstFloats it = src->relatedEntity().floatProperties().begin();
        it != src->relatedEntity().floatProperties().end();
        ++it)
        {
        result.push_back(
          PropertyValuePhrase::create()->setup(it->first, src));
        }
      }
    break;
  case STRING_PROPERTY:
    if (src->relatedEntity().hasStringProperties())
      {
      for (
        PropertyNameWithConstStrings it = src->relatedEntity().stringProperties().begin();
        it != src->relatedEntity().stringProperties().end();
        ++it)
        {
        if (it->first != "name")
          {
          result.push_back(
            PropertyValuePhrase::create()->setup(it->first, src));
          }
        }
      }
    break;
  case INTEGER_PROPERTY:
    if (src->relatedEntity().hasIntegerProperties())
      {
      for (
        PropertyNameWithConstIntegers it = src->relatedEntity().integerProperties().begin();
        it != src->relatedEntity().integerProperties().end();
        ++it)
        {
        result.push_back(
          PropertyValuePhrase::create()->setup(it->first, src));
        }
      }
    break;
  case INVALID_PROPERTY:
  default:
    break;
    }
}

  } // namespace model
} // namespace smtk

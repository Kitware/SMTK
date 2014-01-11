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

template<typename T>
void addEntityPhrases(const T& ents, DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result)
{
  if (static_cast<int>(ents.size()) < limit)
    {
    for (typename T::const_iterator it = ents.begin(); it != ents.end(); ++it)
      {
      result.push_back(
        EntityPhrase::create()->setup(*it, parent));
      }
    }
  else
    {
    result.push_back(
      EntityListPhrase::create()->setup(ents, parent));
    }
}

void SubphraseGenerator::InstancesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  InstanceEntities instances = ent.instances<InstanceEntities>();
  addEntityPhrases(instances, src, this->directLimit(), result);
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
  addEntityPhrases(boundingShells, src, this->directLimit(), result);
}

void SubphraseGenerator::ToplevelShellsOfUse(
  DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result)
{
  ShellEntities toplevelShells = ent.shellEntities<ShellEntities>();
  addEntityPhrases(toplevelShells, src, this->directLimit(), result);
}


void SubphraseGenerator::ToplevelShellsOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  ShellEntities toplevelShells = ent.shellEntities();
  addEntityPhrases(toplevelShells, src, this->directLimit(), result);
}

void SubphraseGenerator::UsesOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  UseEntities cellUses = ent.uses<UseEntities>();
  addEntityPhrases(cellUses, src, this->directLimit(), result);
}

void SubphraseGenerator::InclusionsOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  Cursors inclusions = ent.inclusions<Cursors>();
  addEntityPhrases(inclusions, src, this->directLimit(), result);
}

void SubphraseGenerator::BoundingCellsOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  CellEntities boundingCells = ent.boundingCells();
  addEntityPhrases(boundingCells, src, this->directLimit(), result);
}

void SubphraseGenerator::UsesOfShell(
  DescriptivePhrase::Ptr src, const ShellEntity& ent, DescriptivePhrases& result)
{
  UseEntities shellUses = ent.uses<UseEntities>();
  addEntityPhrases(shellUses, src, this->directLimit(), result);
}


void SubphraseGenerator::MembersOfGroup(
  DescriptivePhrase::Ptr src, const GroupEntity& grp, DescriptivePhrases& result)
{
  CursorArray members = grp.members<CursorArray>();
  addEntityPhrases(members, src, this->directLimit(), result);
  // TODO: Sort by entity type, name, etc.?
}

void SubphraseGenerator::FreeSubmodelsOfModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  ModelEntities freeSubmodelsInModel = mod.submodels();
  addEntityPhrases(freeSubmodelsInModel, src, this->directLimit(), result);
}

void SubphraseGenerator::FreeGroupsInModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  GroupEntities freeGroupsInModel = mod.groups();
  addEntityPhrases(freeGroupsInModel, src, this->directLimit(), result);
}

void SubphraseGenerator::FreeCellsOfModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  CellEntities freeCellsInModel = mod.cells();
  addEntityPhrases(freeCellsInModel, src, this->directLimit(), result);
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
  EntityListPhrase::Ptr src, const CursorArray& ents, DescriptivePhrases& result)
{
  BitFlags commonFlags = INVALID;
  BitFlags unionFlags = 0;

  for (CursorArray::const_iterator it = ents.begin(); it != ents.end(); ++it)
    {
    result.push_back(
      EntityPhrase::create()->setup(*it, src));
    commonFlags &= it->entityFlags();
    unionFlags |= it->entityFlags();
    }
  src->setFlags(commonFlags, unionFlags);
}

void SubphraseGenerator::PropertiesOfPropertyList(
   PropertyListPhrase::Ptr src, PropertyType p, DescriptivePhrases& result)
{
  switch (p)
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

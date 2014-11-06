//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

#include <algorithm>
//required for insert_iterator on VS2010+
#include <iterator>

namespace smtk {
  namespace model {

/**\brief Return a list of descriptive phrases that elaborate upon \a src.
  *
  * Subclasses must override this method.
  */
DescriptivePhrases SubphraseGenerator::subphrases(DescriptivePhrase::Ptr src)
{
  (void)src;
  DescriptivePhrases empty;
  return empty;
}

/**\brief The maximum number of subphrases to directly include before turning into a list.
  *
  * The helper methods in SubphraseGenerator (such as InstancesOfEntity()), will
  * insert small numbers of child items directly into the \a result subphrases.
  * But when more than \a directLimit() items of a given type (such as instances
  * for which the given entity serves as a prototype) are present, a sublist is
  * added a subphrase. This prevents long lists from obscuring other details.
  *
  * Subclasses may override this method.
  */
int SubphraseGenerator::directLimit() const
{
  return 4;
}

/**\brief Should the property of the given type and name be omitted from presentation?
  *
  * Subclasses should override this method.
  */
bool SubphraseGenerator::shouldOmitProperty(
    DescriptivePhrase::Ptr parent, PropertyType ptype, const std::string& pname) const
{
  (void)parent;
  (void)ptype;
  (void)pname;
  return false;
}

void SubphraseGenerator::instancesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  InstanceEntities instances = ent.instances<InstanceEntities>();
  addEntityPhrases(instances, src, this->directLimit(), result);
}

void SubphraseGenerator::attributesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  if (ent.hasAttributes())
    {
    result.push_back(
      AttributeListPhrase::create()->setup(
        ent, ent.attributes(), src));
    }
}

void SubphraseGenerator::propertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  this->stringPropertiesOfEntity(src, ent, result);
  this->integerPropertiesOfEntity(src, ent, result);
  this->floatPropertiesOfEntity(src, ent, result);
}

void SubphraseGenerator::floatPropertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  std::set<std::string> pnames = ent.floatPropertyNames();
  if (!pnames.empty())
    {
    this->addEntityProperties(FLOAT_PROPERTY, pnames, src, result);
    }
}

void SubphraseGenerator::stringPropertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  std::set<std::string> pnames = ent.stringPropertyNames();
  if (!pnames.empty())
    {
    this->addEntityProperties(STRING_PROPERTY, pnames, src, result);
    }
}

void SubphraseGenerator::integerPropertiesOfEntity(
  DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result)
{
  std::set<std::string> pnames = ent.integerPropertyNames();
  if (!pnames.empty())
    {
    this->addEntityProperties(INTEGER_PROPERTY, pnames, src, result);
    }
}


void SubphraseGenerator::cellOfUse(
  DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result)
{
  CellEntity parentCell = ent.cell();
  if (parentCell.isValid())
    {
    result.push_back(
      EntityPhrase::create()->setup(parentCell, src));
    }
}

void SubphraseGenerator::boundingShellsOfUse(
  DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result)
{
  ShellEntities boundingShells = ent.boundingShellEntities<ShellEntities>();
  addEntityPhrases(boundingShells, src, this->directLimit(), result);
}

void SubphraseGenerator::toplevelShellsOfUse(
  DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result)
{
  ShellEntities toplevelShells = ent.shellEntities<ShellEntities>();
  addEntityPhrases(toplevelShells, src, this->directLimit(), result);
}


void SubphraseGenerator::usesOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  UseEntities cellUses = ent.uses<UseEntities>();
  addEntityPhrases(cellUses, src, this->directLimit(), result);
}

void SubphraseGenerator::inclusionsOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  Cursors inclusions = ent.inclusions<Cursors>();
  CellEntities boundingCells = ent.boundingCells();
  Cursors strictInclusions;
  std::set_difference(
    inclusions.begin(), inclusions.end(),
    boundingCells.begin(), boundingCells.end(),
    std::inserter(strictInclusions, strictInclusions.end()));
  addEntityPhrases(strictInclusions, src, this->directLimit(), result);
}

void SubphraseGenerator::boundingCellsOfCell(
  DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result)
{
  CellEntities boundingCells = ent.boundingCells();
  addEntityPhrases(boundingCells, src, this->directLimit(), result);
}

void SubphraseGenerator::usesOfShell(
  DescriptivePhrase::Ptr src, const ShellEntity& ent, DescriptivePhrases& result)
{
  UseEntities shellUses = ent.uses<UseEntities>();
  addEntityPhrases(shellUses, src, this->directLimit(), result);
}


void SubphraseGenerator::membersOfGroup(
  DescriptivePhrase::Ptr src, const GroupEntity& grp, DescriptivePhrases& result)
{
  CursorArray members = grp.members<CursorArray>();
  addEntityPhrases(members, src, this->directLimit(), result);
  // TODO: Sort by entity type, name, etc.?
}

void SubphraseGenerator::freeSubmodelsOfModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  ModelEntities freeSubmodelsInModel = mod.submodels();
  addEntityPhrases(freeSubmodelsInModel, src, this->directLimit(), result);
}

void SubphraseGenerator::freeGroupsInModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  GroupEntities freeGroups = mod.groups();
  addEntityPhrases(freeGroups, src, this->directLimit(), result);
}

void SubphraseGenerator::freeCellsOfModel(
  DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result)
{
  CellEntities freeCellsInModel = mod.cells();
  addEntityPhrases(freeCellsInModel, src, this->directLimit(), result);
}


void SubphraseGenerator::prototypeOfInstance(
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


void SubphraseGenerator::entitiesOfEntityList(
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

void SubphraseGenerator::propertiesOfPropertyList(
   PropertyListPhrase::Ptr src, PropertyType p, DescriptivePhrases& result)
{
  if (src && !src->propertyNames().empty())
    { // We already have a filtered subset of names; use it.
    this->addEntityProperties(p, src->propertyNames(), src, result);
    return;
    }
  // We need to filter the property names.
  std::set<std::string> pnames;
  Cursor ent = src->relatedEntity();
  switch (p)
    {
  case FLOAT_PROPERTY:
    pnames = ent.floatPropertyNames();
    break;
  case STRING_PROPERTY:
    pnames = ent.stringPropertyNames();
    break;
  case INTEGER_PROPERTY:
    pnames = ent.integerPropertyNames();
    break;
  case INVALID_PROPERTY:
  default:
    break;
    }
  if (!pnames.empty())
    {
    this->addEntityProperties(p, pnames, src, result);
    }
}

/// Add subphrases (or a list of them) to \a result for the specified properties.
void SubphraseGenerator::addEntityProperties(
  PropertyType ptype, std::set<std::string>& props,
  DescriptivePhrase::Ptr parnt, DescriptivePhrases& result)
{
  std::set<std::string>::const_iterator it;
  for (it = props.begin(); it != props.end(); )
    {
    std::string pname = *it;
    ++it;
    if (this->shouldOmitProperty(parnt, ptype, pname))
      props.erase(pname);
    }
  // First, use the generator to prune entries we do not wish to display.
  // Now, add either the properties directly or as a list.
  if (static_cast<int>(props.size()) < this->directLimit())
    {
    for (it = props.begin(); it != props.end(); ++it)
      {
      result.push_back(
        PropertyValuePhrase::create()->setup(ptype, *it, parnt));
      }
    }
  else
    {
    result.push_back(
      PropertyListPhrase::create()->setup(
        parnt->relatedEntity(), ptype, props, parnt));
    }
}

  } // namespace model
} // namespace smtk

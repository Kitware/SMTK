//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/Definition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/System.h"

#include <iostream>
#include <sstream>

using namespace smtk::attribute;
double Definition::s_notApplicableBaseColor[4] = {0.0, 0.0, 0.0, 0.0};
double Definition::s_defaultBaseColor[4] = {1.0, 1.0, 1.0, 1.0};

//----------------------------------------------------------------------------
Definition::Definition(
  const std::string &myType,
  smtk::attribute::DefinitionPtr myBaseDef,
  System* mySystem)
{
  this->m_system = mySystem;
  this->m_baseDefinition = myBaseDef;
  this->m_type = myType;
  this->m_version = 0;
  this->m_isAbstract = false;
  this->m_isNodal = false;
  this->m_advanceLevel = 0;
  this->m_isUnique = true;
  this->m_isRequired = false;
  this->m_isNotApplicableColorSet = false;
  this->m_isDefaultColorSet = false;
  if (myBaseDef)
    {
    this->m_baseItemOffset = myBaseDef->numberOfItemDefinitions();
    }
  else
    {
    this->m_baseItemOffset = 0;
    }
}
//----------------------------------------------------------------------------
Definition::~Definition()
{
}
//----------------------------------------------------------------------------
bool Definition::isA(smtk::attribute::ConstDefinitionPtr targetDef) const
{
  // Walk up the inheritence tree until we either hit the root or
  // encounter this definition
  const Definition *def = this;
  for (def = this; def; def = def->m_baseDefinition.get())
    {
    if (def == targetDef.get())
      {
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::conflicts(smtk::attribute::DefinitionPtr def) const
{
  // 2 definitions conflict if their inheritance tree intersects and isUnique is
  // is true within the intersection
  // ASSUMING isUnique has been set consistantly first verify that both definitions
  // are suppose to be unique
  if (!(this->isUnique() && def->isUnique()))
    {
    return false;
    }
  // Test the trivial case that they are the same definition
  if (this == def.get())
    {
    return true;
    }

  // Get the most "basic" definition that is unique
   smtk::attribute::ConstDefinitionPtr baseDef =
     this->m_system->findIsUniqueBaseClass(def);
  // See if the other definition is derived from this base definition.
  // If it is not then we know there is no conflict
  return def->isA(baseDef);
}

/**\brief Return the definition that governs attribute associations.
  *
  * A ModelEntityItemDefinition is used to store information about
  * the allowable associations that may be made between attributes
  * specified by this definition and model entities.
  *
  * The definition's membershipMask() serves as a mask for
  * allowable associations while the definition's minimum and
  * maximum number of associations can be used to indicate whether
  * an association is required, optional, and/or extensible.
  *
  * This method should never return a null shared-pointer;
  * if the rule does not exist, one will be created.
  */
ModelEntityItemDefinitionPtr Definition::associationRule() const
{
  if (!this->m_associationRule)
    {
    std::ostringstream assocName;
    assocName << this->type() << "Associations";
    // We pretend to be const because allocating this object should have
    // no side effect (the newly-allocated definition's defaults should
    // match values returned in its absence).
    Definition* self = const_cast<Definition*>(this);
    self->m_associationRule = ModelEntityItemDefinition::New(assocName.str());
    self->m_associationRule->setMembershipMask(0); // nothing allowed by default.
    // By default, create associations so that once the mask is set
    // there is no limit on the number of entities that may be associated:
    self->m_associationRule->setIsExtensible(true);
    self->m_associationRule->setMaxNumberOfValues(0);
    }
  return this->m_associationRule;
}

/**\brief Set the rule that decides which model entities may be associated with instances of this definition.
  *
  */
void Definition::setAssociationRule(ModelEntityItemDefinitionPtr rule)
{
  this->m_associationRule = rule;
}

/**\brief Return the mask specifying which types of model entities this attribute can be associated with.
  *
  */
smtk::model::BitFlags Definition::associationMask() const
{
  return !! this->m_associationRule ? this->m_associationRule->membershipMask() : 0;
}

/**\brief Set the mask specifying which types of model entities this attribute can be associated with.
  *
  */
void Definition::setAssociationMask(smtk::model::BitFlags mask)
{
  if (mask)
    this->associationRule()->setMembershipMask(mask);
  else
    this->m_associationRule = ModelEntityItemDefinitionPtr();
}

/// Returns whether this attribute can be associated with vertices.
bool Definition::associatesWithVertex() const
{
  return smtk::model::isVertex(this->associationMask());
}

/// Returns whether this attribute can be associated with edges.
bool Definition::associatesWithEdge() const
{
  return smtk::model::isEdge(this->associationMask());
}

/// Returns whether this attribute can be associated with faces.
bool Definition::associatesWithFace() const
{
  return smtk::model::isFace(this->associationMask());
}

/// Returns whether this attribute can be associated with volumes.
bool Definition::associatesWithVolume() const
{
  return smtk::model::isVolume(this->associationMask());
}

/// Returns whether this attribute can be associated with models.
bool Definition::associatesWithModel() const
{
  return smtk::model::isModel(this->associationMask());
}

/// Returns whether this attribute can be associated with groups.
bool Definition::associatesWithGroup() const
{
  return smtk::model::isGroup(this->associationMask());
}

/// Return whether this attribute can be associated with entities that have the given \a flag value.
bool Definition::canBeAssociated(smtk::model::BitFlags flag) const
{
  // This is a simpler version of Group::meetsMembershipConstraintsInternal().
  smtk::model::BitFlags mask = this->associationMask();
  smtk::model::BitFlags memberTest = flag & mask;
  if (!(memberTest & smtk::model::ANY_DIMENSION) && (mask & smtk::model::ANY_DIMENSION)) return false;
  if (!(memberTest & smtk::model::ENTITY_MASK) && (mask & smtk::model::ENTITY_MASK)) return false;
  return true;
}

/**\brief Return whether this attribute can be associated with the given \a entity.
  *
  * TODO:
  *   In this case we need to process BCS and DS specially
  *   We look at the model's dimension and based on that return
  *   the appropriate associatesWith method
  *   Conflicts will contain a list of attributes that prevent an attribute
  *   of this type from being associated
  */
bool
Definition::canBeAssociated(smtk::model::EntityRef /*entity*/,
                            std::vector<Attribute *>* /*inConflicts*/) const
{
  // TO DO - Need to pull in Model Entity class to do this
  // Procedure:
  // 1. Determine if the definition can be applied to the model entity - this
  // involves getting its type and calling the appropriate associatesWith method.
  // In the case of a boundary condition set or domain set we need to look at the
  // model's dimension to call the appropriate method. - return false if it can't.
  // 2. Get a list of attributes on the entity and call conflicts method with each
  // definition.  All conflicting attributes gets added to the list.
  return false;
}
//----------------------------------------------------------------------------
void Definition::buildAttribute(Attribute *att) const
{
  // If there is a super definition have it prep the attribute and add its items
  const Definition *bdef = this->m_baseDefinition.get();
  if (bdef)
    {
    bdef->buildAttribute(att);
    }
  else
    {
    // This is the "base definition" so first we should make sure the attribute
    // is "empty" of items and associations
    att->removeAllItems();
    att->m_associations = ModelEntityItemPtr();
    }

  // If the definition allows associations, create an item to hold them:
  if (this->associationMask())
    {
    att->m_associations =
      smtk::dynamic_pointer_cast<ModelEntityItem>(
        this->associationRule()->buildItem(att, -2));
    att->m_associations->setDefinition(this->associationRule());
    }

  // Next - for each item definition we have build and add the appropriate
  // item to the attribute
  smtk::attribute::ItemPtr comp;
  std::size_t i, j, n = this->m_itemDefs.size();
  j = att->numberOfItems();
  for (i = 0; i < n; i++, j++)
    {
    comp = this->m_itemDefs[i]->buildItem(att, static_cast<int>(j));
    comp->setDefinition(this->m_itemDefs[i]);
    att->addItem(comp);
    }
}
//----------------------------------------------------------------------------
bool Definition::isMemberOf(const std::vector<std::string> &inCategories) const
{
  std::size_t i, n = inCategories.size();
  for (i = 0; i < n; i++)
    {
    if (this->isMemberOf(inCategories[i]))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->findItemPosition(cdef->name()) >= 0)
    {
    return false;
    }
  std::size_t n = this->m_itemDefs.size();
  this->m_itemDefs.push_back(cdef);
  this->m_itemDefPositions[cdef->name()] = static_cast<int>(n);
  this->updateDerivedDefinitions();
  return true;
}
//----------------------------------------------------------------------------
void Definition::updateDerivedDefinitions()
{
  DefinitionPtr def = this->pointer();
  if (def)
    {
    this->m_system->updateDerivedDefinitionIndexOffsets(def);
    }
}
//----------------------------------------------------------------------------
void Definition::setCategories()
{
  if (this->m_baseDefinition)
    {
    this->m_categories = this->m_baseDefinition->m_categories;
    }
  else
    {
    this->m_categories.clear();
    }
  std::size_t i, n = this->m_itemDefs.size();
  for (i = 0; i < n; i++)
    {
    this->m_itemDefs[i]->updateCategories();
    const std::set<std::string> &itemCats = this->m_itemDefs[i]->categories();
    this->m_categories.insert(itemCats.begin(), itemCats.end());
    }
}
//----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr Definition::pointer() const
{
  System *m = this->system();
  if (m)
    {
    return m->findDefinition(this->m_type);
    }
  return smtk::attribute::DefinitionPtr();
}
//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr Definition::itemDefinition(int ith) const
{
  // Is the item in this defintion?
  if (ith >= static_cast<int>(this->m_baseItemOffset))
    {
    return this->m_itemDefs[static_cast<std::size_t>(ith-this->m_baseItemOffset)];
    }
  else if (this->m_baseDefinition)
    {
    return this->m_baseDefinition->itemDefinition(ith);
    }
  return smtk::attribute::ItemDefinitionPtr();
}
 //----------------------------------------------------------------------------
int Definition::findItemPosition(const std::string &name) const
{
  std::map<std::string, int>::const_iterator it;
  it = this->m_itemDefPositions.find(name);
  if (it == this->m_itemDefPositions.end())
    {
    // Check the base definition if there is one
    if (this->m_baseDefinition)
      {
      return this->m_baseDefinition->findItemPosition(name);
      }
    else
      {
      return -1; // named item doesn't exist
      }
    }
  return it->second + static_cast<int>(this->m_baseItemOffset);
}
//----------------------------------------------------------------------------

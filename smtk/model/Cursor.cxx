//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Cursor.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/DefaultBridge.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Events.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Tessellation.h"

#include <boost/functional/hash.hpp>

#include <algorithm>

using namespace smtk::common;

namespace smtk {
  namespace model {

/// Construct an invalid cursor.
Cursor::Cursor()
{
}

/// Construct a cursor referencing a given \a entity residing in the given \a mgr.
Cursor::Cursor(ManagerPtr mgr, const smtk::common::UUID& inEntity)
  : m_manager(mgr), m_entity(inEntity)
{
}

/// Change the underlying manager the cursor references.
bool Cursor::setManager(ManagerPtr mgr)
{
  if (mgr == this->m_manager.lock())
    {
    return false;
    }
  this->m_manager = mgr;
  return true;
}

/// Return the underlying manager the cursor references.
ManagerPtr Cursor::manager()
{
  return this->m_manager.lock();
}

/// Return the underlying manager the cursor references.
const ManagerPtr Cursor::manager() const
{
  return this->m_manager.lock();
}

/// Change the UUID of the entity the cursor references.
bool Cursor::setEntity(const smtk::common::UUID& inEntity)
{
  if (inEntity == this->m_entity)
    return false;

  this->m_entity = inEntity;
  return true;
}

/// Return the UUID of the entity the cursor references.
const smtk::common::UUID& Cursor::entity() const
{
  return this->m_entity;
}

/**\brief Return the nominal parametric dimension of the entity (or -1).
  *
  * A value of -1 is returned when the cursor is invalid, or the entity
  * does not have any specified dimension, or the entity may have components
  * with multiple distinct dimensionalities.
  */
int Cursor::dimension() const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    Entity* entRec = mgr->findEntity(this->m_entity);
    if (entRec)
      {
      return entRec->dimension();
      }
    }
  return -1;
}

/**\brief Return the nominal parametric dimension(s) of the entity as a bit vector.
  *
  * A value of 0 is returned when the cursor is invalid.
  * Multiple bits will be set if the entity may have components with
  * multiple distinct dimensionalities.
  */
int Cursor::dimensionBits() const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    Entity* entRec = mgr->findEntity(this->m_entity);
    if (entRec)
      {
      return entRec->dimensionBits();
      }
    }
  return 0;
}

/**\brief Set the dimensionality of this entity. WARNING: Intended for internal use only.
  *
  * This method is used by some bridges to change the dimensionality of a model
  * after it has been transcribed. Do not call it.
  */
void Cursor::setDimensionBits(BitFlags dimBits)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    Entity* entRec = mgr->findEntity(this->m_entity);
    if (entRec)
      {
      BitFlags old = entRec->entityFlags() & ~ANY_DIMENSION;
      entRec->setEntityFlags(old | dimBits);
      }
    }
}

/**\brief Return the maximum number of coordinates required to parameterize this model's point locus.
  *
  * Unlike Cursor::dimension(), this will always return a non-negative number
  * for valid cell, use, shell and model entities.
  * It returns -1 for unpopulated groups with no members, instance entities, bridge sessions,
  * and other entities with no dimension bits set.
  */
int Cursor::maxParametricDimension() const
{
  int result = -1;
  BitFlags dimbits = this->dimensionBits();
  if (dimbits == 0) return result;
  BitFlags onedim = DIMENSION_0;
  while (1)
    {
    ++result;
    if (2 * onedim > dimbits)
      return result;
    onedim <<= 1;
    }
  return -1;
}

/**\brief Return the dimension of the space into which this entity is embedded.
  *
  * This is the number of actual coordinate values associated with points in
  * the underlying space of the entity (i.e., the point locus that make it up).
  * It should be equal to or greater than the maximum parametric dimension.
  *
  * All the entities in a model must have the same embedding dimension; it
  * is a property stored with the model.
  *
  * By default, the embedding dimension is 3.
  */
int Cursor::embeddingDimension() const
{
  ModelEntity owner = this->owningModel();
  if (owner.isValid() && owner.hasIntegerProperty("embedding dimension"))
    {
    const IntegerList& prop(owner.integerProperty("embedding dimension"));
    if (!prop.empty())
      return prop[0];
    }
  return this->maxParametricDimension();
}

/// Return the bit vector describing the entity's type. \sa isVector, isEdge, ...
BitFlags Cursor::entityFlags() const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    Entity* entRec = mgr->findEntity(this->m_entity);
    if (entRec)
      {
      return entRec->entityFlags();
      }
    }
  return 0;
}

/**\brief A string summary of the type of entity represented by this cursor.
  *
  * If \a form is non-zero, the plural form of the summary is returned.
  */
std::string Cursor::flagSummary(int form) const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr)
    {
    Entity* ent = mgr->findEntity(this->m_entity);
    if (ent)
      {
      std::ostringstream summary;
      // We can embellish the entity BitFlag summary with
      // additional information for some objects.
      if (ent->entityFlags() & BRIDGE_SESSION)
        {
        Bridge::Ptr brdg = mgr->findBridgeSession(this->m_entity);
        if (brdg)
          {
          // if this is a DefaultBridge and there is a remote bridge name, display that;
          // otherwise, show the local bridge name.
          DefaultBridgePtr defaultBr = smtk::dynamic_pointer_cast<DefaultBridge>(brdg);
          if(defaultBr && !defaultBr->remoteName().empty())
            summary << defaultBr->remoteName() << " ";
          else
            summary << brdg->name() << " ";
          }
        }
      summary << ent->flagSummary(form);
      return summary.str();
      }
    }
  return Entity::flagSummary(INVALID, form);
}

/** Report the name associated with this entity.
  *
  * This will not assign a default name to the entity (that would
  * violate the const-ness of the method), but will report a
  * UUID-based name for the entity if it does not have a
  * user-assigned name.
  */
std::string Cursor::name() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr ? mgr->name(this->m_entity) : "(null model)";
}

/** Assign a name to an entity.
  *
  * This will override any existing name.
  */
void Cursor::setName(const std::string& n)
{
  this->setStringProperty("name", n);
}

/**\brief Assign a default name to the entity.
  *
  * This uses counters associated with the owning
  * model or model manager to name the entity.
  */
std::string Cursor::assignDefaultName()
{
  ManagerPtr mgr = this->m_manager.lock();
  if (!mgr || !this->m_entity)
    return std::string();

  return mgr->assignDefaultName(this->m_entity);
}

/// Returns true if the "visible" integer-property exists.
bool Cursor::hasVisibility() const
{
  return this->hasIntegerProperty("visible");
}

/** Report the visibility associated with this entity.
  *
  * If there is no "visible" property set,return false
  */
bool Cursor::visible() const
{
  if ( !this->hasVisibility() )
    return false;

  const IntegerList& prop(this->integerProperty("visible"));
  return (!prop.empty() && (prop[0] != 0));
}

/** Assign the visible property to an entity.
  *
  * This will override any existing visibility.
  */
void Cursor::setVisible(bool vis)
{
  this->setIntegerProperty("visible", vis ? 1 : 0);
}

/** Return a user-assigned color for the entity.
  *
  * If no color was assigned, (0,0,0,0) will be returned.
  */
FloatList Cursor::color() const
{
  FloatList result = this->floatProperty("color");
  int ncomp = static_cast<int>(result.size());
  if (ncomp < 4)
    {
    result.resize(4);
    for (int i = ncomp; i < 3; ++i)
      result[i] = 0.;
    switch (ncomp)
      {
    default:
    case 0: // Assuming color not defined; mark alpha invalid.
      result[3] = -1.;
      break;
    case 1:
    case 3: // Assume RGB or greyscale; default alpha = 1.
      result[3] = 1.;
      break;
    case 2: // Assume greyscale+alpha; remap alpha to result[4]
      result[3] = (result[1] >= 0. && result[1] <= 0. ? result[1] : 1.);
      break;
      }
    }
  return result;
}

/// Returns true if the "color" float-property exists. No check on the size is performed.
bool Cursor::hasColor() const
{
  return this->hasFloatProperty("color");
}

/** Assign a color to an entity.
  *
  * This will override any existing color.
  * No check on the size is performed, but you should provide 4 values per color:
  * red, green, blue, and alpha. Each should be in [0,1].
  */
void Cursor::setColor(const smtk::model::FloatList& rgba)
{
  this->setFloatProperty("color", rgba);
}

/** Assign a color to an entity.
  *
  * This will override any existing color.
  * Each value should be in [0,1].
  */
void Cursor::setColor(double red, double green, double blue, double alpha)
{
  FloatList rgba;
  rgba.resize(4);
  rgba[0] = red;
  rgba[1] = green;
  rgba[2] = blue;
  rgba[3] = alpha;
  this->setColor(rgba);
}

/**\brief Return whether the cursor is pointing to valid manager that contains the UUID of the entity.
  *
  * Subclasses should not override this method. It is a convenience
  * which makes the shiboken wrapper more functional.
  */
bool Cursor::isValid() const
{
  return this->isValid(NULL);
}

/**\brief Return whether the cursor is pointing to valid manager that contains the UUID of the entity.
  *
  * Subclasses override this and additionally return whether the entity is of
  * a type that matches the Cursor subclass. For example, it is possible to
  * create a Vertex cursor from a UUID referring to an EdgeUse. While
  * Cursor::isValid() will return true, Vertex::isValid() will return false.
  *
  * The optional \a entityRecord will be set when a non-NULL value is passed
  * and the entity is valid.
  */
bool Cursor::isValid(Entity** entityRecord) const
{
  ManagerPtr mgr = this->m_manager.lock();
  bool status = mgr && !this->m_entity.isNull();
  if (status)
    {
    Entity* rec = mgr->findEntity(this->m_entity);
    status = rec ? true : false;
    if (status && entityRecord)
      {
      *entityRecord = rec;
      }
    }
  return status;
}

/**\brief A wrapper around Cursor::isValid() which also verifies an arrangement exists.
  *
  */
bool Cursor::checkForArrangements(ArrangementKind k, Entity*& entRec, Arrangements*& arr) const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (this->isValid(&entRec))
    {
    arr = NULL;
    if (
      (arr = mgr->hasArrangementsOfKindForEntity(this->m_entity, k)) &&
      !arr->empty())
      {
      return true;
      }
    }
  return false;
}

Cursors Cursor::bordantEntities(int ofDimension) const
{
  Cursors result;
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    smtk::common::UUIDs uids = mgr->bordantEntities(
      this->m_entity, ofDimension);
    CursorsFromUUIDs(result, mgr, uids);
    }
  return result;
}

Cursors Cursor::boundaryEntities(int ofDimension) const
{
  Cursors result;
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    smtk::common::UUIDs uids = mgr->boundaryEntities(
      this->m_entity, ofDimension);
    CursorsFromUUIDs(result, mgr, uids);
    }
  return result;
}

Cursors Cursor::lowerDimensionalBoundaries(int lowerDimension)
{
  Cursors result;
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    smtk::common::UUIDs uids = mgr->lowerDimensionalBoundaries(
      this->m_entity, lowerDimension);
    CursorsFromUUIDs(result, mgr, uids);
    }
  return result;
}

Cursors Cursor::higherDimensionalBordants(int higherDimension)
{
  Cursors result;
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    smtk::common::UUIDs uids = mgr->higherDimensionalBordants(
      this->m_entity, higherDimension);
    CursorsFromUUIDs(result, mgr, uids);
    }
  return result;
}

Cursors Cursor::adjacentEntities(int ofDimension)
{
  Cursors result;
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    smtk::common::UUIDs uids = mgr->adjacentEntities(
      this->m_entity, ofDimension);
    CursorsFromUUIDs(result, mgr, uids);
    }
  return result;
}

/// Return a set of entities related to this entity. This method is provided for Python wrapping.
Cursors Cursor::relations() const
{
  return this->relationsAs<Cursors>();
}

/**\brief Add a relation to an entity, \a ent, without specifying the relationship's nature.
  *
  * The relation is considered "raw" because no arrangement information is added
  * describing the nature of the arrangement.
  *
  * This method adds a relation regardless of whether \a ent is already a relation;
  * \a ent may appear in the entity's list of relations multiple times after this call.
  */
Cursor& Cursor::addRawRelation(const Cursor& ent)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (
    mgr &&
    !this->m_entity.isNull() &&
    mgr == ent.manager() &&
    !ent.entity().isNull() &&
    ent.entity() != this->m_entity)
    {
    Entity* entRec = mgr->findEntity(this->m_entity);
    if (entRec)
      entRec->appendRelation(ent.entity());
    }
  return *this;
}

/**\brief Find or add a relation to an entity, \a ent, without specifying the relationship's nature.
  *
  * The relation is considered "raw" because no arrangement information is added
  * describing the nature of the arrangement.
  *
  * This method has no effect if \a ent is already a relation.
  */
Cursor& Cursor::findOrAddRawRelation(const Cursor& ent)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (
    mgr &&
    !this->m_entity.isNull() &&
    mgr == ent.manager() &&
    !ent.entity().isNull() &&
    ent.entity() != this->m_entity)
    {
    Entity* entRec = mgr->findEntity(this->m_entity);
    if (
      entRec &&
      std::find(
        entRec->relations().begin(),
        entRec->relations().end(),
        ent.entity())
      == entRec->relations().end())
      entRec->appendRelation(ent.entity());
    }
  return *this;
}

/**\brief Return the entity's tessellation if one exists or NULL otherwise.
  *
  */
const Tessellation* Cursor::hasTessellation() const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    UUIDsToTessellations::const_iterator it = mgr->tessellations().find(this->m_entity);
    if (it != mgr->tessellations().end())
      return &it->second;
    }
  return NULL;
}

/** @name Attribute associations
  *
  */
///@{
/**\brief Does the cursor have any attributes associated with it?
  */
bool Cursor::hasAttributes() const
{
  ManagerPtr mgr = this->m_manager.lock();
  UUIDsToAttributeAssignments::const_iterator it =
    mgr->attributeAssignments().find(this->m_entity);
  if (it != mgr->attributeAssignments().end())
    {
    return it->second.attributes().empty() ? false : true;
    }
  return false;
}

/**\brief Does the cursor have any attributes associated with it?
 */
    bool Cursor::hasAttribute(const smtk::common::UUID &attribId) const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->hasAttribute(attribId, this->m_entity);
}

/**\brief Does the cursor have any attributes associated with it?
  */
bool Cursor::associateAttribute(const smtk::common::UUID &attribId)
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->associateAttribute(attribId, this->m_entity);
}

/**\brief Does the cursor have any attributes associated with it?
  */
bool Cursor::disassociateAttribute(const smtk::common::UUID &attribId, bool reverse)
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->disassociateAttribute(attribId, this->m_entity, reverse);
}

/**\brief Does the cursor have any attributes associated with it?
  */
AttributeAssignments& Cursor::attributes()
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->attributeAssignments()[this->m_entity];
}
/**\brief Does the cursor have any attributes associated with it?
  */
AttributeSet Cursor::attributes() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->attributeAssignments()[this->m_entity].attributes();
}
///@}

void Cursor::setFloatProperty(const std::string& propName, smtk::model::Float propValue)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    mgr->setFloatProperty(this->m_entity, propName, propValue);
    }
}

void Cursor::setFloatProperty(const std::string& propName, const smtk::model::FloatList& propValue)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    mgr->setFloatProperty(this->m_entity, propName, propValue);
    }
}

smtk::model::FloatList const& Cursor::floatProperty(const std::string& propName) const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->floatProperty(this->m_entity, propName);
}

smtk::model::FloatList& Cursor::floatProperty(const std::string& propName)
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->floatProperty(this->m_entity, propName);
}

bool Cursor::hasFloatProperty(const std::string& propName) const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    return mgr->hasFloatProperty(this->m_entity, propName);
    }
  return false;
}

bool Cursor::removeFloatProperty(const std::string& propName)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    return mgr->removeFloatProperty(this->m_entity, propName);
    }
  return false;
}

/**\brief Does this entity have *any* float-valued properties?
  *
  * If not, you should call Cursor::setFloatProperty() before
  * calling Cursor::floatProperties().
  */
bool Cursor::hasFloatProperties() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return
    mgr->floatProperties().find(this->m_entity)
    == mgr->floatProperties().end() ?
    false : true;
}

/// Return the names of all the floating-point properties.
std::set<std::string> Cursor::floatPropertyNames() const
{
  std::set<std::string> pnames;
  if (this->hasFloatProperties())
    {
    const FloatData& props(this->floatProperties());
    for (FloatData::const_iterator it = props.begin(); it != props.end(); ++it)
      {
      pnames.insert(it->first);
      }
    }
  return pnames;
}

FloatData& Cursor::floatProperties()
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->floatProperties().find(this->m_entity)->second;
}

FloatData const& Cursor::floatProperties() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->floatProperties().find(this->m_entity)->second;
}


void Cursor::setStringProperty(const std::string& propName, const smtk::model::String& propValue)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    mgr->setStringProperty(this->m_entity, propName, propValue);
    }
}

void Cursor::setStringProperty(const std::string& propName, const smtk::model::StringList& propValue)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    mgr->setStringProperty(this->m_entity, propName, propValue);
    }
}

smtk::model::StringList const& Cursor::stringProperty(const std::string& propName) const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->stringProperty(this->m_entity, propName);
}

smtk::model::StringList& Cursor::stringProperty(const std::string& propName)
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->stringProperty(this->m_entity, propName);
}

bool Cursor::hasStringProperty(const std::string& propName) const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    return mgr->hasStringProperty(this->m_entity, propName);
    }
  return false;
}

bool Cursor::removeStringProperty(const std::string& propName)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    return mgr->removeStringProperty(this->m_entity, propName);
    }
  return false;
}

/**\brief Does this entity have *any* string-valued properties?
  *
  * If not, you should call Cursor::setStringProperty() before
  * calling Cursor::stringProperties().
  */
bool Cursor::hasStringProperties() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return
    mgr->stringProperties().find(this->m_entity)
    == mgr->stringProperties().end() ?
    false : true;
}

/// Return the names of all the string properties.
std::set<std::string> Cursor::stringPropertyNames() const
{
  std::set<std::string> pnames;
  if (this->hasStringProperties())
    {
    const StringData& props(this->stringProperties());
    for (StringData::const_iterator it = props.begin(); it != props.end(); ++it)
      {
      pnames.insert(it->first);
      }
    }
  return pnames;
}

StringData& Cursor::stringProperties()
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->stringProperties().find(this->m_entity)->second;
}

StringData const& Cursor::stringProperties() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->stringProperties().find(this->m_entity)->second;
}


void Cursor::setIntegerProperty(const std::string& propName, smtk::model::Integer propValue)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    mgr->setIntegerProperty(this->m_entity, propName, propValue);
    }
}

void Cursor::setIntegerProperty(const std::string& propName, const smtk::model::IntegerList& propValue)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    mgr->setIntegerProperty(this->m_entity, propName, propValue);
    }
}

smtk::model::IntegerList const& Cursor::integerProperty(const std::string& propName) const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->integerProperty(this->m_entity, propName);
}

smtk::model::IntegerList& Cursor::integerProperty(const std::string& propName)
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->integerProperty(this->m_entity, propName);
}

bool Cursor::hasIntegerProperty(const std::string& propName) const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    return mgr->hasIntegerProperty(this->m_entity, propName);
    }
  return false;
}

bool Cursor::removeIntegerProperty(const std::string& propName)
{
  ManagerPtr mgr = this->m_manager.lock();
  if (mgr && !this->m_entity.isNull())
    {
    return mgr->removeIntegerProperty(this->m_entity, propName);
    }
  return false;
}

/**\brief Does this entity have *any* integer-valued properties?
  *
  * If not, you should call Cursor::setIntegerProperty() before
  * calling Cursor::integerProperties().
  */
bool Cursor::hasIntegerProperties() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return
    mgr->integerProperties().find(this->m_entity)
    == mgr->integerProperties().end() ?
    false : true;
}

/// Return the names of all the integer properties.
std::set<std::string> Cursor::integerPropertyNames() const
{
  std::set<std::string> pnames;
  if (this->hasIntegerProperties())
    {
    const IntegerData& props(this->integerProperties());
    for (IntegerData::const_iterator it = props.begin(); it != props.end(); ++it)
      {
      pnames.insert(it->first);
      }
    }
  return pnames;
}

IntegerData& Cursor::integerProperties()
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->integerProperties().find(this->m_entity)->second;
}

IntegerData const& Cursor::integerProperties() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->integerProperties().find(this->m_entity)->second;
}

/// Return the number of arrangements of the given kind \a k.
int Cursor::numberOfArrangementsOfKind(ArrangementKind k) const
{
  ManagerPtr mgr = this->m_manager.lock();
  const Arrangements* arr =
    mgr->hasArrangementsOfKindForEntity(
      this->m_entity, k);
  return arr ? static_cast<int>(arr->size()) : 0;
}

/// Return the \a i-th arrangement of kind \a k (or NULL).
Arrangement* Cursor::findArrangement(ArrangementKind k, int i)
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->findArrangement(this->m_entity, k, i);
}

/// Return the \a i-th arrangement of kind \a k (or NULL).
const Arrangement* Cursor::findArrangement(ArrangementKind k, int i) const
{
  ManagerPtr mgr = this->m_manager.lock();
  return mgr->findArrangement(this->m_entity, k, i);
}

/**\brief Return the relation specified by the \a offset into the specified arrangement.
  *
  */
Cursor Cursor::relationFromArrangement(
  ArrangementKind k, int arrangementIndex, int offset) const
{
  ManagerPtr mgr = this->m_manager.lock();
  const Entity* ent = mgr->findEntity(this->m_entity);
  if (ent)
    {
    const Arrangement* arr = this->findArrangement(k, arrangementIndex);
    if (arr && static_cast<int>(arr->details().size()) > offset)
      {
      int idx = arr->details()[offset];
      return idx < 0 ?
        Cursor() :
        Cursor(mgr, ent->relations()[idx]);
      }
    }
  return Cursor();
}

/**\brief Embed the specified \a thingToEmbed as an inclusion into this cursor's entity.
  *
  * This adds an INCLUDES relation (if necessary) to this entity and
  * an EMBEDDED_IN relation (if necessary) to the \a thingToEmbed.
  */
Cursor& Cursor::embedEntity(const Cursor& thingToEmbed)
{
  ManagerPtr mgr = this->m_manager.lock();
  //ManagerEventType event = std::make_pair(ADD_EVENT, INVALID_RELATIONSHIP);
  ManagerEventType event = std::make_pair(ADD_EVENT, this->embeddingRelationType(thingToEmbed));
  if (event.second != INVALID_RELATIONSHIP)
    {
    CursorArrangementOps::findOrAddSimpleRelationship(*this, INCLUDES, thingToEmbed);
    CursorArrangementOps::findOrAddSimpleRelationship(thingToEmbed, EMBEDDED_IN, *this);
    mgr->trigger(event, *this, thingToEmbed);
    }
  return *this;
}

/**\brief Unembed the specified \a thingToUnembed as an inclusion into this cursor's entity.
  *
  * This removes an INCLUDES relation (if necessary) to this entity and
  * an EMBEDDED_IN relation (if necessary) to the \a thingToUnembed.
  */
Cursor& Cursor::unembedEntity(const Cursor& thingToEmbed)
{
  ManagerPtr mgr = this->m_manager.lock();
  ManagerEventType event = std::make_pair(DEL_EVENT, this->embeddingRelationType(thingToEmbed));
  if (event.second != INVALID_RELATIONSHIP)
    {
    int aidx = CursorArrangementOps::findSimpleRelationship(*this, INCLUDES, thingToEmbed);
    if (aidx >= 0)
      {
      mgr->unarrangeEntity(this->m_entity, EMBEDDED_IN, aidx);
      mgr->trigger(event, *this, thingToEmbed);
      }
    }
  return *this;
}

/**\brief Return whether the specified \a entity is a direct inclusion in this cursor's entity.
  *
  */
bool Cursor::isEmbedded(Cursor& ent) const
{
  return CursorArrangementOps::findSimpleRelationship(*this, INCLUDES, ent) >= 0;
}

/**\brief Report the entity into which this entity is directly embedded.
  *
  * If the entity is not embedded into another, the result will be an invalid cursor.
  */
Cursor Cursor::embeddedIn() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, EMBEDDED_IN);
}

/**\brief Return the ModelEntity which owns this entity.
  *
  * A ModelEntity is analogous to a body in some kernels,
  * a lump in others, or a group of regions in yet others.
  * In SMTK, models are the objects that get associated with
  * bridges to other kernels, so determining the owning
  * model is important for discovering which modeling operations
  * (including reading and writing to/from disk) are possible.
  *
  * Note that the returned model may be invalid.
  */
ModelEntity Cursor::owningModel() const
{
  ManagerPtr mgr = this->m_manager.lock();
  return ModelEntity(
    mgr,
    mgr->modelOwningEntity(this->m_entity));
}

/// A comparator provided so that cursors may be included in ordered sets.
bool Cursor::operator == (const Cursor& other) const
{
  ManagerPtr mgr = this->m_manager.lock();
  return (
    (mgr == other.manager()) &&
    (this->m_entity == other.m_entity)) ?
    true :
    false;
}

/// A comparator provided so that cursors may be included in ordered sets.
bool Cursor::operator < (const Cursor& other) const
{
  ManagerPtr mgr = this->m_manager.lock();
  ManagerPtr otherMgr = other.m_manager.lock();
  if (mgr < otherMgr)
    {
    return true;
    }
  else if (otherMgr < mgr)
    {
    return false;
    }
  return this->m_entity < other.m_entity;
}

/**\brief A hash function for cursors.
  *
  * This allows cursors to be put into sets in Python.
  */
std::size_t Cursor::hash() const
{
  ManagerPtr mgr = this->m_manager.lock();
  std::size_t result = this->m_entity.hash();
  boost::hash_combine(result, reinterpret_cast<std::size_t>(mgr.get()));
  return result;
}

ManagerEventRelationType Cursor::embeddingRelationType(const Cursor& embedded) const
{
  ManagerEventRelationType reln = INVALID_RELATIONSHIP;

  switch (this->entityFlags() & ENTITY_MASK)
    {
  case MODEL_ENTITY:
    switch (embedded.entityFlags() & ENTITY_MASK)
      {
    case SHELL_ENTITY: reln = MODEL_INCLUDES_FREE_SHELL; break;
    case CELL_ENTITY: reln = MODEL_INCLUDES_FREE_CELL; break;
    case USE_ENTITY: reln = MODEL_INCLUDES_FREE_USE; break;
      }
    break;
  case CELL_ENTITY:
    switch (embedded.entityFlags() & ENTITY_MASK)
      {
    case CELL_ENTITY: reln = CELL_INCLUDES_CELL; break;
      }
    break;
  case SHELL_ENTITY:
    switch (embedded.entityFlags() & ENTITY_MASK)
      {
    case SHELL_ENTITY: reln = SHELL_INCLUDES_SHELL; break;
    case USE_ENTITY: reln = SHELL_HAS_USE; break;
      }
    break;
    }

  return reln;
}

std::ostream& operator << (std::ostream& os, const Cursor& c)
{
  os << c.name();
  return os;
}

std::size_t cursorHash(const Cursor& c)
{
  return c.hash();
}

/*! \fn template<typename T> T Cursor::relationsAs() const
 *\brief Return all of the entities related to this cursor.
 *
 * The return value is a template parameter naming container type.
 * Each member is cast from a Cursor to T::value_type and added
 * to the container only if valid. This makes it possible to
 * subset the relations by forcing them into a container of the
 * desired type.
 */

/*! \fn template<typename S, typename T> void Cursor::CursorsFromUUIDs(S& result, ManagerPtr mgr, const T& uids)
 *\brief Convert a set of UUIDs into a set of cursors referencing the same \a mgr.
 */

/*! \fn template<typename T> Cursor::embedEntities(const T& container)
 * \brief Embed each of the entities in the container inside this entity.
 */

/*! \fn Cursor::instances() const
 * \brief Return all the instances this object serves as a prototype for.
 */

/*! \fn Cursor::properties<T>()
 *  \brief Return a pointer to the properties of the entity, creating an entry as required.
 *
 * Unlike the hasProperties() method, this will return a valid pointer as long as the
 * manager and entity of the cursor are valid.
 * If the entity does not already have any properties of the given type, a new
 * StringData, FloatData, or IntegerData instance is created and added to the
 * appropriate map.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template<>
StringData* Cursor::properties<StringData>()
{
  if (!this->hasStringProperties())
    {
    if (!this->manager() || !this->entity())
      return NULL;
    StringData blank;
    this->manager()->stringProperties().insert(
      std::pair<UUID,StringData>(this->entity(), blank));
    }
  return &(this->stringProperties());
}

template<>
FloatData* Cursor::properties<FloatData>()
{
  if (!this->hasFloatProperties())
    {
    if (!this->manager() || !this->entity())
      return NULL;
    FloatData blank;
    this->manager()->floatProperties().insert(
      std::pair<UUID,FloatData>(this->entity(), blank));
    }
  return &(this->floatProperties());
}

template<>
IntegerData* Cursor::properties<IntegerData>()
{
  if (!this->hasIntegerProperties())
    {
    if (!this->manager() || !this->entity())
      return NULL;
    IntegerData blank;
    this->manager()->integerProperties().insert(
      std::pair<UUID,IntegerData>(this->entity(), blank));
    }
  return &(this->integerProperties());
}

/*! \fn Cursor::hasProperties<T>() const
 *! \fn Cursor::hasProperties<T>()
 *  \brief Return a pointer to the properties of the entity or null if none exist.
 *
 * Unlike the properties() method, this will return a NULL pointer
 * if the entity does not already have any properties of the given type.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template<>
StringData* Cursor::hasProperties<StringData>()
{
  if (this->hasStringProperties())
    return &(this->stringProperties());
  return NULL;
}

template<>
const StringData* Cursor::hasProperties<StringData>() const
{
  if (this->hasStringProperties())
    return &(this->stringProperties());
  return NULL;
}

template<>
FloatData* Cursor::hasProperties<FloatData>()
{
  if (this->hasFloatProperties())
    return &(this->floatProperties());
  return NULL;
}

template<>
const FloatData* Cursor::hasProperties<FloatData>() const
{
  if (this->hasFloatProperties())
    return &(this->floatProperties());
  return NULL;
}

template<>
IntegerData* Cursor::hasProperties<IntegerData>()
{
  if (this->hasIntegerProperties())
    return &(this->integerProperties());
  return NULL;
}

template<>
const IntegerData* Cursor::hasProperties<IntegerData>() const
{
  if (this->hasIntegerProperties())
    return &(this->integerProperties());
  return NULL;
}

/*! \fn Cursor::removeProperty<T>(const std::string& name)
 *  \brief Remove the property of type \a T with the given \a name, returning true on success.
 *
 * False is returned if the property did not exist for the given entity.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template<>
bool Cursor::removeProperty<StringData>(const std::string& pname)
{ return this->removeStringProperty(pname); }

template<>
bool Cursor::removeProperty<FloatData>(const std::string& pname)
{ return this->removeFloatProperty(pname); }

template<>
bool Cursor::removeProperty<IntegerData>(const std::string& pname)
{ return this->removeIntegerProperty(pname); }

  } // namespace model
} // namespace smtk

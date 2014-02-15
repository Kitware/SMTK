#include "smtk/model/Cursor.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

/// Construct an invalid cursor.
Cursor::Cursor()
{
}

/// Construct a cursor referencing a given \a entity residing in the given \a storage.
Cursor::Cursor(StoragePtr inStorage, const smtk::util::UUID& inEntity)
  : m_storage(inStorage), m_entity(inEntity)
{
}

/// Change the underlying storage the cursor references.
bool Cursor::setStorage(StoragePtr inStorage)
{
  if (inStorage == this->m_storage)
    {
    return false;
    }
  this->m_storage = inStorage;
  return true;
}

/// Return the underlying storage the cursor references.
StoragePtr Cursor::storage()
{
  return this->m_storage;
}

/// Return the underlying storage the cursor references.
const StoragePtr Cursor::storage() const
{
  return this->m_storage;
}

/// Change the UUID of the entity the cursor references.
bool Cursor::setEntity(const smtk::util::UUID& inEntity)
{
  if (inEntity == this->m_entity)
    {
    return false;
    }
  this->m_entity = inEntity;
  return true;
}

/// Return the UUID of the entity the cursor references.
const smtk::util::UUID& Cursor::entity() const
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
  if (this->m_storage && !this->m_entity.isNull())
    {
    Entity* entRec = this->m_storage->findEntity(this->m_entity);
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
  if (this->m_storage && !this->m_entity.isNull())
    {
    Entity* entRec = this->m_storage->findEntity(this->m_entity);
    if (entRec)
      {
      return entRec->dimensionBits();
      }
    }
  return 0;
}

/// Return the bit vector describing the entity's type. \sa isVector, isEdge, ...
BitFlags Cursor::entityFlags() const
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    Entity* entRec = this->m_storage->findEntity(this->m_entity);
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
  Entity* ent = this->m_storage->findEntity(this->m_entity);
  if (ent)
    {
    return ent->flagSummary(form);
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
  return this->m_storage->name(this->m_entity);
}

/** Assign a name to an entity.
  *
  * This will override any existing name.
  */
void Cursor::setName(const std::string& n)
{
  this->setStringProperty("name", n);
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

/**\brief Return whether the cursor is pointing to valid storage that contains the UUID of the entity.
  *
  * Subclasses should not override this method. It is a convenience
  * which makes the shiboken wrapper more functional.
  */
bool Cursor::isValid() const
{
  return this->isValid(NULL);
}

/**\brief Return whether the cursor is pointing to valid storage that contains the UUID of the entity.
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
  bool status = this->m_storage && !this->m_entity.isNull();
  if (status)
    {
    Entity* rec = this->m_storage->findEntity(this->m_entity);
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
  if (this->isValid(&entRec))
    {
    arr = NULL;
    if (
      (arr = this->m_storage->hasArrangementsOfKindForEntity(this->m_entity, k)) &&
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
  if (this->m_storage && !this->m_entity.isNull())
    {
    smtk::util::UUIDs uids = this->m_storage->bordantEntities(
      this->m_entity, ofDimension);
    CursorsFromUUIDs(result, this->m_storage, uids);
    }
  return result;
}

Cursors Cursor::boundaryEntities(int ofDimension) const
{
  Cursors result;
  if (this->m_storage && !this->m_entity.isNull())
    {
    smtk::util::UUIDs uids = this->m_storage->boundaryEntities(
      this->m_entity, ofDimension);
    CursorsFromUUIDs(result, this->m_storage, uids);
    }
  return result;
}

Cursors Cursor::lowerDimensionalBoundaries(int lowerDimension)
{
  Cursors result;
  if (this->m_storage && !this->m_entity.isNull())
    {
    smtk::util::UUIDs uids = this->m_storage->lowerDimensionalBoundaries(
      this->m_entity, lowerDimension);
    CursorsFromUUIDs(result, this->m_storage, uids);
    }
  return result;
}

Cursors Cursor::higherDimensionalBordants(int higherDimension)
{
  Cursors result;
  if (this->m_storage && !this->m_entity.isNull())
    {
    smtk::util::UUIDs uids = this->m_storage->higherDimensionalBordants(
      this->m_entity, higherDimension);
    CursorsFromUUIDs(result, this->m_storage, uids);
    }
  return result;
}

Cursors Cursor::adjacentEntities(int ofDimension)
{
  Cursors result;
  if (this->m_storage && !this->m_entity.isNull())
    {
    smtk::util::UUIDs uids = this->m_storage->adjacentEntities(
      this->m_entity, ofDimension);
    CursorsFromUUIDs(result, this->m_storage, uids);
    }
  return result;
}

/// Add a relation to an entity, \a ent, with no arrangement information.
Cursor& Cursor::addRawRelation(const Cursor& ent)
{
  if (
    this->m_storage &&
    !this->m_entity.isNull() &&
    this->m_storage == ent.storage() &&
    !ent.entity().isNull() &&
    ent.entity() != this->m_entity)
    {
    Entity* entRec = this->m_storage->findEntity(this->m_entity);
    if (entRec)
      {
      entRec->appendRelation(ent.entity());
      }
    }
  return *this;
}

/**\brief Return the entity's tessellation if one exists or NULL otherwise.
  *
  */
const Tessellation* Cursor::hasTessellation() const
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    UUIDsToTessellations::const_iterator it = this->m_storage->tessellations().find(this->m_entity);
    if (it != this->m_storage->tessellations().end())
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
  UUIDsToAttributeAssignments::const_iterator it =
    this->m_storage->attributeAssignments().find(this->m_entity);
  if (it != this->m_storage->attributeAssignments().end())
    {
    return it->second.attributes().empty() ? false : true;
    }
  return false;
}

/**\brief Does the cursor have any attributes associated with it?
  */
bool Cursor::hasAttribute(int attribId) const
{
  return this->m_storage->hasAttribute(attribId, this->m_entity);
}

/**\brief Does the cursor have any attributes associated with it?
  */
bool Cursor::attachAttribute(int attribId)
{
  return this->m_storage->attachAttribute(attribId, this->m_entity);
}

/**\brief Does the cursor have any attributes associated with it?
  */
bool Cursor::detachAttribute(int attribId, bool reverse)
{
  return this->m_storage->detachAttribute(attribId, this->m_entity, reverse);
}

/**\brief Does the cursor have any attributes associated with it?
  */
AttributeAssignments& Cursor::attributes()
{
  return this->m_storage->attributeAssignments()[this->m_entity];
}
/**\brief Does the cursor have any attributes associated with it?
  */
AttributeSet Cursor::attributes() const
{
  return this->m_storage->attributeAssignments()[this->m_entity].attributes();
}
///@}

void Cursor::setFloatProperty(const std::string& propName, smtk::model::Float propValue)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    this->m_storage->setFloatProperty(this->m_entity, propName, propValue);
    }
}

void Cursor::setFloatProperty(const std::string& propName, const smtk::model::FloatList& propValue)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    this->m_storage->setFloatProperty(this->m_entity, propName, propValue);
    }
}

smtk::model::FloatList const& Cursor::floatProperty(const std::string& propName) const
{
  return this->m_storage->floatProperty(this->m_entity, propName);
}

smtk::model::FloatList& Cursor::floatProperty(const std::string& propName)
{
  return this->m_storage->floatProperty(this->m_entity, propName);
}

bool Cursor::hasFloatProperty(const std::string& propName) const
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    return this->m_storage->hasFloatProperty(this->m_entity, propName);
    }
  return false;
}

bool Cursor::removeFloatProperty(const std::string& propName)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    return this->m_storage->removeFloatProperty(this->m_entity, propName);
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
  return
    this->m_storage->floatProperties().find(this->m_entity)
    == this->m_storage->floatProperties().end() ?
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
  return this->m_storage->floatProperties().find(this->m_entity)->second;
}

FloatData const& Cursor::floatProperties() const
{
  return this->m_storage->floatProperties().find(this->m_entity)->second;
}


void Cursor::setStringProperty(const std::string& propName, const smtk::model::String& propValue)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    this->m_storage->setStringProperty(this->m_entity, propName, propValue);
    }
}

void Cursor::setStringProperty(const std::string& propName, const smtk::model::StringList& propValue)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    this->m_storage->setStringProperty(this->m_entity, propName, propValue);
    }
}

smtk::model::StringList const& Cursor::stringProperty(const std::string& propName) const
{
  return this->m_storage->stringProperty(this->m_entity, propName);
}

smtk::model::StringList& Cursor::stringProperty(const std::string& propName)
{
  return this->m_storage->stringProperty(this->m_entity, propName);
}

bool Cursor::hasStringProperty(const std::string& propName) const
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    return this->m_storage->hasStringProperty(this->m_entity, propName);
    }
  return false;
}

bool Cursor::removeStringProperty(const std::string& propName)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    return this->m_storage->removeStringProperty(this->m_entity, propName);
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
  return
    this->m_storage->stringProperties().find(this->m_entity)
    == this->m_storage->stringProperties().end() ?
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
  return this->m_storage->stringProperties().find(this->m_entity)->second;
}

StringData const& Cursor::stringProperties() const
{
  return this->m_storage->stringProperties().find(this->m_entity)->second;
}


void Cursor::setIntegerProperty(const std::string& propName, smtk::model::Integer propValue)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    this->m_storage->setIntegerProperty(this->m_entity, propName, propValue);
    }
}

void Cursor::setIntegerProperty(const std::string& propName, const smtk::model::IntegerList& propValue)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    this->m_storage->setIntegerProperty(this->m_entity, propName, propValue);
    }
}

smtk::model::IntegerList const& Cursor::integerProperty(const std::string& propName) const
{
  return this->m_storage->integerProperty(this->m_entity, propName);
}

smtk::model::IntegerList& Cursor::integerProperty(const std::string& propName)
{
  return this->m_storage->integerProperty(this->m_entity, propName);
}

bool Cursor::hasIntegerProperty(const std::string& propName) const
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    return this->m_storage->hasIntegerProperty(this->m_entity, propName);
    }
  return false;
}

bool Cursor::removeIntegerProperty(const std::string& propName)
{
  if (this->m_storage && !this->m_entity.isNull())
    {
    return this->m_storage->removeIntegerProperty(this->m_entity, propName);
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
  return
    this->m_storage->integerProperties().find(this->m_entity)
    == this->m_storage->integerProperties().end() ?
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
  return this->m_storage->integerProperties().find(this->m_entity)->second;
}

IntegerData const& Cursor::integerProperties() const
{
  return this->m_storage->integerProperties().find(this->m_entity)->second;
}

/// Return the number of arrangements of the given kind \a k.
int Cursor::numberOfArrangementsOfKind(ArrangementKind k) const
{
  const Arrangements* arr =
    this->m_storage->hasArrangementsOfKindForEntity(
      this->m_entity, k);
  return arr ? static_cast<int>(arr->size()) : 0;
}

/// Return the \a i-th arrangement of kind \a k (or NULL).
Arrangement* Cursor::findArrangement(ArrangementKind k, int i)
{
  return this->m_storage->findArrangement(this->m_entity, k, i);
}

/// Return the \a i-th arrangement of kind \a k (or NULL).
const Arrangement* Cursor::findArrangement(ArrangementKind k, int i) const
{
  return this->m_storage->findArrangement(this->m_entity, k, i);
}

/**\brief Return the relation specified by the \a offset into the specified arrangement.
  *
  */
Cursor Cursor::relationFromArrangement(
  ArrangementKind k, int arrangementIndex, int offset) const
{
  const Entity* ent = this->m_storage->findEntity(this->m_entity);
  if (ent)
    {
    const Arrangement* arr = this->findArrangement(k, arrangementIndex);
    if (arr && static_cast<int>(arr->details().size()) > offset)
      {
      int idx = arr->details()[offset];
      return idx < 0 ?
        Cursor() :
        Cursor(this->m_storage, ent->relations()[idx]);
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
  CursorArrangementOps::findOrAddSimpleRelationship(*this, INCLUDES, thingToEmbed);
  CursorArrangementOps::findOrAddSimpleRelationship(thingToEmbed, EMBEDDED_IN, *this);
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

/// A comparator provided so that cursors may be included in ordered sets.
bool Cursor::operator == (const Cursor& other) const
{
  return (
    (this->m_storage == other.m_storage) &&
    (this->m_entity == other.m_entity)) ?
    true :
    false;
}

/// A comparator provided so that cursors may be included in ordered sets.
bool Cursor::operator < (const Cursor& other) const
{
  if (this->m_storage < other.m_storage)
    {
    return true;
    }
  else if (other.m_storage < this->m_storage)
    {
    return false;
    }
  return this->m_entity < other.m_entity;
}

std::ostream& operator << (std::ostream& os, const Cursor& c)
{
  os << c.name();
  return os;
}

/*! \fn template<typename S, typename T> void Cursor::CursorsFromUUIDs(S& result, StoragePtr storage, const T& uids)
 *\brief Convert a set of UUIDs into a set of cursors referencing the same \a storage.
 */

/*! \fn template<typename T> Cursor::embedEntities(const T& container)
 * \brief Embed each of the entities in the container inside this entity.
 */

/*! \fn Cursor::instances() const
 * \brief Return all the instances this object serves as a prototype for.
 */

  } // namespace model
} // namespace smtk

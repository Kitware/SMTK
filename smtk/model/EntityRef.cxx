//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EntityRef.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Volume.h"

#include "smtk/geometry/Geometry.h"

#include <boost/functional/hash.hpp>
#include <cfloat>

#include <algorithm>

using namespace smtk::common;

namespace smtk
{
namespace model
{

/// Construct an invalid entityref.
EntityRef::EntityRef() = default;

/// Construct a entityref referencing a given \a entity residing in the given \a rsrc.
EntityRef::EntityRef(ResourcePtr rsrc, const smtk::common::UUID& inEntity)
  : m_resource(rsrc)
  , m_entity(inEntity)
{
}

/// Construct a entityref referencing a given \a entity residing in the given \a rsrc.
EntityRef::EntityRef(EntityPtr src)
{
  if (src)
  {
    m_resource = src->modelResource();
    m_entity = src->id();
  }
}

/// Change the underlying resource the entityref references.
bool EntityRef::setResource(ResourcePtr rsrc)
{
  if (rsrc == m_resource.lock())
  {
    return false;
  }
  m_resource = rsrc;
  return true;
}

/// Return the underlying resource the entityref references.
ResourcePtr EntityRef::resource()
{
  return m_resource.lock();
}

/// Return the underlying resource the entityref references.
const ResourcePtr EntityRef::resource() const
{
  return m_resource.lock();
}

/// Change the UUID of the entity the entityref references.
bool EntityRef::setEntity(const smtk::common::UUID& inEntity)
{
  if (inEntity == m_entity)
    return false;

  m_entity = inEntity;
  return true;
}

/// Return the UUID of the entity the entityref references.
const smtk::common::UUID& EntityRef::entity() const
{
  return m_entity;
}

/// Return the smtk::model::Entity record for this model entity.
smtk::model::EntityPtr EntityRef::entityRecord() const
{
  auto rsrc = this->resource();
  if (!rsrc)
  {
    return nullptr;
  }
  return rsrc->findEntity(m_entity, true);
}

/// Return the resource component for this model entity.
smtk::resource::ComponentPtr EntityRef::component() const
{
  auto rsrc = this->resource();
  if (!rsrc)
  {
    return nullptr;
  }
  return rsrc->find(m_entity);
}

/**\brief Return the nominal parametric dimension of the entity (or -1).
  *
  * A value of -1 is returned when the entityref is invalid, or the entity
  * does not have any specified dimension, or the entity may have components
  * with multiple distinct dimensionalities.
  */
int EntityRef::dimension() const
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    EntityPtr entRec = rsrc->findEntity(m_entity);
    if (entRec)
    {
      return entRec->dimension();
    }
  }
  return -1;
}

/**\brief Return the nominal parametric dimension(s) of the entity as a bit vector.
  *
  * A value of 0 is returned when the entityref is invalid.
  * Multiple bits will be set if the entity may have components with
  * multiple distinct dimensionalities.
  */
int EntityRef::dimensionBits() const
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    EntityPtr entRec = rsrc->findEntity(m_entity);
    if (entRec)
    {
      return entRec->dimensionBits();
    }
  }
  return 0;
}

/**\brief Set the dimensionality of this entity. WARNING: Intended for internal use only.
  *
  * This method is used by some sessions to change the dimensionality of a model
  * after it has been transcribed. Do not call it.
  */
void EntityRef::setDimensionBits(BitFlags dimBits)
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    EntityPtr entRec = rsrc->findEntity(m_entity);
    if (entRec)
    {
      BitFlags old = entRec->entityFlags() & ~ANY_DIMENSION;
      entRec->setEntityFlags(old | dimBits);
    }
  }
}

/**\brief Return the maximum number of coordinates required to parameterize this model's point locus.
  *
  * Unlike EntityRef::dimension(), this will always return a non-negative number
  * for valid cell, use, shell and model entities.
  * It returns -1 for unpopulated groups with no members, instance entities, sessions,
  * and other entities with no dimension bits set.
  */
int EntityRef::maxParametricDimension() const
{
  int result = -1;
  BitFlags dimbits = this->dimensionBits();
  if (dimbits == 0)
    return result;
  BitFlags onedim = DIMENSION_0;
  while (true)
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
int EntityRef::embeddingDimension() const
{
  Model owner = this->owningModel();
  if (owner.isValid() && owner.hasIntegerProperty("embedding dimension"))
  {
    const IntegerList& prop(owner.integerProperty("embedding dimension"));
    if (!prop.empty())
      return prop[0];
  }
  return this->maxParametricDimension();
}

/// Return the bit vector describing the entity's type. \sa isVector, isEdge, ...
BitFlags EntityRef::entityFlags() const
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    EntityPtr entRec = rsrc->findEntity(m_entity);
    if (entRec)
    {
      return entRec->entityFlags();
    }
  }
  return 0;
}

/**\brief A string summary of the type of entity represented by this entityref.
  *
  * If \a form is non-zero, the plural form of the summary is returned.
  */
std::string EntityRef::flagSummary(int form) const
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc)
  {
    EntityPtr ent = rsrc->findEntity(m_entity);
    if (ent)
    {
      std::ostringstream summary;
      // We can embellish the entity BitFlag summary with
      // additional information for some objects.
      if (ent->entityFlags() & SESSION)
      {
        Session::Ptr brdg = rsrc->sessionData(*this);
        if (brdg)
        {
          // if this is a DefaultSession and there is a remote session name, display that;
          // otherwise, show the local session name.
          DefaultSessionPtr defaultBr = smtk::dynamic_pointer_cast<DefaultSession>(brdg);
          if (defaultBr && !defaultBr->remoteName().empty())
            summary << defaultBr->remoteName() << " ";
          else
            summary << brdg->name() << " ";
        }
      }
      if (this->hasStringProperty("_type"))
      {
        std::string plural = form ? "s" : "";
        summary << this->stringProperty("_type")[0] << plural;
      }
      else
      {
        summary << ent->flagSummary(form);
      }
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
std::string EntityRef::name() const
{
  ResourcePtr rsrc = m_resource.lock();
  return rsrc ? rsrc->name(m_entity) : "(null model)";
}

/** Assign a name to an entity.
  *
  * This will override any existing name.
  */
void EntityRef::setName(const std::string& n)
{
  this->setStringProperty("name", n);
}

/**\brief Assign a default name to the entity.
  *
  * This uses counters associated with the owning
  * model or model resource to name the entity.
  */
std::string EntityRef::assignDefaultName(bool overwrite)
{
  ResourcePtr rsrc = m_resource.lock();
  if (!rsrc || !m_entity)
    return std::string();

  return overwrite ? rsrc->assignDefaultName(m_entity) : rsrc->assignDefaultNameIfMissing(m_entity);
}

void EntityRef::setExclusions(bool v, int mask)
{
  // Get curent hiddenOptions
  int exclusionOps = this->hasIntegerProperty("exclusion") ? this->exclusions() : 0;
  if (mask == Exclusions::Everything)
  { // Either set every bits to 1 or 0
    exclusionOps = v ? Exclusions::Everything : 0;
  }
  else
  {
    if (v)
    {
      exclusionOps |= mask;
    }
    else
    {
      exclusionOps &= ~mask;
    }
  }

  this->setIntegerProperty("exclusion", exclusionOps);
}

int EntityRef::exclusions(int mask) const
{
  if (!this->hasIntegerProperty("exclusion") || this->integerProperty("exclusion").empty())
  {
    return 0;
  }

  const IntegerList& prop(this->integerProperty("exclusion"));
  int value = prop.empty() ? 0 : (static_cast<int>(prop[0]) & mask);
  return mask == Exclusions::Everything ? value : !!value;
}

/// Returns true if the "visible" integer-property exists.
bool EntityRef::hasVisibility() const
{
  return this->hasIntegerProperty("visible");
}

/** Report the visibility associated with this entity.
  *
  * If there is no "visible" property set,return true
  */
bool EntityRef::visible() const
{
  if (!this->hasVisibility())
    return true;

  const IntegerList& prop(this->integerProperty("visible"));
  return (!prop.empty() && (prop[0] != 0));
}

/** Assign the visible property to an entity.
  *
  * This will override any existing visibility.
  */
void EntityRef::setVisible(bool vis)
{
  this->setIntegerProperty("visible", vis ? 1 : 0);
}

/** Return a user-assigned color for the entity.
  *
  * If no color was assigned, (0,0,0,-1) will be returned.
  */
FloatList EntityRef::color() const
{
  FloatList result = !this->hasColor() ? FloatList{} : this->floatProperty("color");
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
bool EntityRef::hasColor() const
{
  return this->hasFloatProperty("color");
}

/** Assign a color to an entity.
  *
  * This will override any existing color.
  * No check on the size is performed, but you should provide 4 values per color:
  * red, green, blue, and alpha. Each should be in [0,1].
  */
void EntityRef::setColor(const smtk::model::FloatList& rgba)
{
  this->setFloatProperty("color", rgba);
}

/** Assign a color to an entity.
  *
  * This will override any existing color.
  * Each value should be in [0,1].
  */
void EntityRef::setColor(double red, double green, double blue, double alpha)
{
  FloatList rgba;
  rgba.resize(4);
  rgba[0] = red;
  rgba[1] = green;
  rgba[2] = blue;
  rgba[3] = alpha;
  this->setColor(rgba);
}

/**\brief Return whether the entityref is pointing to valid resource that contains the UUID of the entity.
  *
  * Subclasses should not override this method.
  */
bool EntityRef::isValid() const
{
  return this->isValid(nullptr);
}

/**\brief Return whether the entityref is pointing to valid resource that contains the UUID of the entity.
  *
  * Subclasses override this and additionally return whether the entity is of
  * a type that matches the EntityRef subclass. For example, it is possible to
  * create a Vertex entityref from a UUID referring to an EdgeUse. While
  * EntityRef::isValid() will return true, Vertex::isValid() will return false.
  *
  * The optional \a entityRecord will be set when a non-nullptr value is passed
  * and the entity is valid.
  */
bool EntityRef::isValid(EntityPtr* entityRecord) const
{
  ResourcePtr rsrc = m_resource.lock();
  bool status = rsrc && !m_entity.isNull();
  if (status)
  {
    EntityPtr rec = rsrc->findEntity(m_entity, false);
    status = static_cast<bool>(rec);
    if (status && entityRecord)
    {
      *entityRecord = rec;
    }
  }
  return status;
}

/**\brief A wrapper around EntityRef::isValid() which also verifies an arrangement exists.
  *
  */
bool EntityRef::checkForArrangements(ArrangementKind k, EntityPtr& entRec, Arrangements*& arr) const
{
  ResourcePtr rsrc = m_resource.lock();
  if (this->isValid(&entRec))
  {
    arr = nullptr;
    if ((arr = rsrc->hasArrangementsOfKindForEntity(m_entity, k)) && !arr->empty())
    {
      return true;
    }
  }
  return false;
}

EntityRefs EntityRef::bordantEntities(int ofDimension) const
{
  EntityRefs result;
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    smtk::common::UUIDs uids = rsrc->bordantEntities(m_entity, ofDimension);
    EntityRefsFromUUIDs(result, rsrc, uids);
  }
  return result;
}

EntityRefs EntityRef::boundaryEntities(int ofDimension) const
{
  EntityRefs result;
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    smtk::common::UUIDs uids = rsrc->boundaryEntities(m_entity, ofDimension);
    EntityRefsFromUUIDs(result, rsrc, uids);
  }
  return result;
}

std::vector<double> EntityRef::boundingBox() const
{
  std::vector<double> bBox, dummy;
  // initialize the BBox, following VTK's rule
  for (int i = 0; i < 6; i++)
  {
    double tmp = (i % 2 == 1) ? -DBL_MAX : DBL_MAX;
    bBox.push_back(tmp);
    dummy.push_back(tmp);
  }

  ResourcePtr rsrc = m_resource.lock();
  if (!rsrc)
  {
    return dummy;
  }

  auto ent = this->entityRecord();
  if (!ent)
  {
    return dummy;
  }

  auto& geom = rsrc->geometry();

  std::array<double, 6> bounds;
  if (this->isCellEntity())
  {
    // Generally only volumes do not have boundingBox as float property
    if (this->hasFloatProperty(SMTK_BOUNDING_BOX_PROP)) // vertex/edge/face
    {
      smtk::model::FloatList currentBBox(this->floatProperty(SMTK_BOUNDING_BOX_PROP));
      bBox = this->unionBoundingBox(bBox, currentBBox);
    }
    else if (
      geom && geom->generationNumber(ent) != smtk::geometry::Geometry::Invalid) // vertex/edge/face
    {
      geom->bounds(ent, bounds);
      bBox = this->unionBoundingBox(bBox, bounds);
      return bBox;
    }
    else
    {
      // loop over children and set bbox
      EntityRefs children = this->boundaryEntities(this->dimension() - 1);
      for (EntityRefs::iterator child = children.begin(); child != children.end(); ++child)
      {
        auto childEnt = child->entityRecord();
        if (child->hasFloatProperty(SMTK_BOUNDING_BOX_PROP))
        {
          bBox = this->unionBoundingBox(bBox, child->floatProperty(SMTK_BOUNDING_BOX_PROP));
        }
        else if (geom && geom->generationNumber(childEnt) != smtk::geometry::Geometry::Invalid)
        {
          geom->bounds(childEnt, bounds);
          bBox = this->unionBoundingBox(bBox, bounds);
        }
      }
      return bBox;
    }
  }
  else if (this->isModel())
  {
    smtk::model::Model model = this->as<Model>();
    if (!model.isValid())
    {
      return bBox;
    }

    // loop over cells and set bbox
    CellEntities cellEnts = model.cells();
    for (CellEntities::iterator cellEnt = cellEnts.begin(); cellEnt != cellEnts.end(); ++cellEnt)
    {
      if (cellEnt->isValid())
      {
        bBox = this->unionBoundingBox(bBox, cellEnt->boundingBox());
      }
    }
    // loop over submodels and set bbox
    Models submodels = model.submodels();
    for (Models::iterator subModel = submodels.begin(); subModel != submodels.end(); ++subModel)
    {
      if (subModel->isValid())
      {
        bBox = this->unionBoundingBox(bBox, subModel->boundingBox());
      }
    }
    return bBox;
  }
  else if (this->isGroup())
  {

    if (this->hasFloatProperty(SMTK_BOUNDING_BOX_PROP)) // exodus session
    {
      smtk::model::FloatList currentBBox(this->floatProperty(SMTK_BOUNDING_BOX_PROP));
      bBox = this->unionBoundingBox(bBox, currentBBox);
      return bBox;
    }

    smtk::model::Group group = this->as<Group>();
    EntityRefs members = group.members<EntityRefs>();
    for (EntityRefs::iterator member = members.begin(); member != members.end(); ++member)
    {
      if (member->isValid())
      {
        bBox = this->unionBoundingBox(bBox, member->boundingBox());
      }
    }
    return bBox;
  }
  else if (this->isAuxiliaryGeometry())
  {
    if (this->hasFloatProperty(SMTK_BOUNDING_BOX_PROP))
    {
      smtk::model::FloatList currentBBox(this->floatProperty(SMTK_BOUNDING_BOX_PROP));
      bBox = this->unionBoundingBox(bBox, currentBBox);
      return bBox;
    }
    return dummy;
  }
  else
  {
    return dummy;
  }
  return bBox;
}

std::vector<double> EntityRef::unionBoundingBox(
  const std::vector<double>& b1,
  const std::vector<double>& b2) const
{
  std::vector<double> resultBBox;
  resultBBox.push_back(std::min(b1[0], b2[0]));
  resultBBox.push_back(std::max(b1[1], b2[1]));
  resultBBox.push_back(std::min(b1[2], b2[2]));
  resultBBox.push_back(std::max(b1[3], b2[3]));
  resultBBox.push_back(std::min(b1[4], b2[4]));
  resultBBox.push_back(std::max(b1[5], b2[5]));
  return resultBBox;
}

std::vector<double> EntityRef::unionBoundingBox(
  const std::vector<double>& b1,
  const std::array<double, 6>& b2) const
{
  std::vector<double> resultBBox;
  resultBBox.push_back(std::min(b1[0], b2[0]));
  resultBBox.push_back(std::max(b1[1], b2[1]));
  resultBBox.push_back(std::min(b1[2], b2[2]));
  resultBBox.push_back(std::max(b1[3], b2[3]));
  resultBBox.push_back(std::min(b1[4], b2[4]));
  resultBBox.push_back(std::max(b1[5], b2[5]));
  return resultBBox;
}

EntityRefs EntityRef::lowerDimensionalBoundaries(int lowerDimension)
{
  EntityRefs result;
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    smtk::common::UUIDs uids = rsrc->lowerDimensionalBoundaries(m_entity, lowerDimension);
    EntityRefsFromUUIDs(result, rsrc, uids);
  }
  return result;
}

EntityRefs EntityRef::higherDimensionalBordants(int higherDimension)
{
  EntityRefs result;
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    smtk::common::UUIDs uids = rsrc->higherDimensionalBordants(m_entity, higherDimension);
    EntityRefsFromUUIDs(result, rsrc, uids);
  }
  return result;
}

EntityRefs EntityRef::adjacentEntities(int ofDimension)
{
  EntityRefs result;
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    smtk::common::UUIDs uids = rsrc->adjacentEntities(m_entity, ofDimension);
    EntityRefsFromUUIDs(result, rsrc, uids);
  }
  return result;
}

/// Return a set of entities related to this entity. This method is provided for Python wrapping.
EntityRefs EntityRef::relations() const
{
  return this->relationsAs<EntityRefs>();
}

/**\brief Add a relation to an entity, \a ent, without specifying the relationship's nature.
  *
  * The relation is considered "raw" because no arrangement information is added
  * describing the nature of the arrangement.
  *
  * This method adds a relation regardless of whether \a ent is already a relation;
  * \a ent may appear in the entity's list of relations multiple times after this call.
  */
EntityRef& EntityRef::addRawRelation(const EntityRef& ent)
{
  ResourcePtr rsrc = m_resource.lock();
  if (
    rsrc && !m_entity.isNull() && rsrc == ent.resource() && !ent.entity().isNull() &&
    ent.entity() != m_entity)
  {
    EntityPtr entRec = rsrc->findEntity(m_entity);
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
EntityRef& EntityRef::findOrAddRawRelation(const EntityRef& ent)
{
  ResourcePtr rsrc = m_resource.lock();
  if (
    rsrc && !m_entity.isNull() && rsrc == ent.resource() && !ent.entity().isNull() &&
    ent.entity() != m_entity)
  {
    EntityPtr entRec = rsrc->findEntity(m_entity);
    if (
      entRec &&
      std::find(entRec->relations().begin(), entRec->relations().end(), ent.entity()) ==
        entRec->relations().end())
      entRec->appendRelation(ent.entity());
  }
  return *this;
}

/**\brief Nullify a relation to the given entity, \a ent.
  *
  * Any matching entries to \a ent in this entity's list of relations
  * is set to the null UUID. This is done (as opposed to removing the
  * relation) so that indices into the list of relations are preserved
  * for any UUIDs that appear after \a ent.
  *
  * The relation is considered "raw" because no arrangement information
  * (which would describe the nature of the arrangement)
  * is presumed to exist.
  *
  * This method has no effect if \a ent is unrelated.
  */
EntityRef& EntityRef::elideRawRelation(const EntityRef& ent)
{
  ResourcePtr rsrc = m_resource.lock();
  if (
    rsrc && !m_entity.isNull() && rsrc == ent.resource() && !ent.entity().isNull() &&
    ent.entity() != m_entity)
  {
    UUIDWithEntityPtr entRec = rsrc->topology().find(m_entity);
    if (entRec != rsrc->topology().end())
    {
      rsrc->elideOneEntityReference(entRec, ent.entity());
    }
  }
  return *this;
}
/**\brief Return an empty tessellation attached to the entity.
  *
  * NB. You are responsible for calling setBoundingBox() on the entity after the
  *     resulting tessellation has been updated.
  */
Tessellation* EntityRef::resetTessellation()
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    UUIDsToTessellations::iterator it = rsrc->tessellations().find(m_entity);
    if (it != rsrc->tessellations().end())
    {
      // Reset the tessellation and increment the generation number
      it->second.reset();
      this->setIntegerProperty(SMTK_TESS_GEN_PROP, this->tessellationGeneration() + 1);
      return &it->second;
    }
  }
  // No existing tessellation
  Tessellation blank;
  UUIDsToTessellations::iterator tessit = rsrc->setTessellation(m_entity, blank);
  return &tessit->second;
}

/**\brief Return the entity's tessellation if one exists or nullptr otherwise.
  *
  */
const Tessellation* EntityRef::hasTessellation() const
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    UUIDsToTessellations::const_iterator it = rsrc->tessellations().find(m_entity);
    if (it != rsrc->tessellations().end())
    {
      return &it->second;
    }
    // But wait, there's more! Instances store rules that can be used to
    // generate a tessellation. So we should try that before giving up:
    if (this->isInstance())
    {
      return this->as<Instance>().generateTessellation();
    }
  }
  return nullptr;
}

/**\brief Return the entity's analysis mesh if one exists or nullptr otherwise.
  *
  */
const Tessellation* EntityRef::hasAnalysisMesh() const
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    UUIDsToTessellations::const_iterator it = rsrc->analysisMesh().find(m_entity);
    if (it != rsrc->analysisMesh().end())
      return &it->second;
  }
  return nullptr;
}

/**\brief Return the entity's analysis mesh if one exists,
  * otherwise return the entity's tessellation. If neither
  * exist return nullptr.
  *
  */
const Tessellation* EntityRef::gotMesh() const
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    UUIDsToTessellations::const_iterator am = rsrc->analysisMesh().find(m_entity);
    if (am != rsrc->analysisMesh().end())
      return &am->second;
    //don't even search for the tessellation if we have an analysis mesh
    UUIDsToTessellations::const_iterator te = rsrc->tessellations().find(m_entity);
    if (te != rsrc->tessellations().end())
      return &te->second;
  }
  return nullptr;
}

/**\brief Set the tessellation of the entity, returning its
  *       generation number (or -1 on failure).
  *
  * If \a analysisMesh is non-zero (zero is the default), then
  * the tessellation is treated as an analysis mesh rather than
  * a tessellation for display purposes.
  *
  * This should not be called outside of an operator.
  */
int EntityRef::setTessellation(const Tessellation* tess, int analysisMesh, bool updateBBox)
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
  {
    int gen;
    try
    {
      if (updateBBox)
      {
        rsrc->setTessellationAndBoundingBox(m_entity, *tess, analysisMesh, &gen);
      }
      else
      {
        rsrc->setTessellation(m_entity, *tess, analysisMesh, &gen);
      }
    }
    catch (std::string& badIdMsg)
    {
      (void)badIdMsg;
      return -1;
    }

    return gen;
  }
  return -1;
}

/**\brief Remove the tessellation of the entity, returning true
  *       if such a tessellation existed.
  *
  * If \a removeGen is true (false is the default), then
  * also remove the generation-number property (a monotonically
  * increasing integer property used to signal that the
  * tessellation has changed).
  */
bool EntityRef::removeTessellation(bool removeGen)
{
  ResourcePtr rsrc = m_resource.lock();
  if (rsrc && !m_entity.isNull())
    return rsrc->removeTessellation(m_entity, removeGen);
  return false;
}

/**\brief Return the generation number of the current tessellation (or -1 if no tessellation).
  *
  */
int EntityRef::tessellationGeneration() const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& integerProperties = comp->properties().get<std::vector<long>>();
    if (
      !integerProperties.contains(SMTK_TESS_GEN_PROP) ||
      integerProperties.at(SMTK_TESS_GEN_PROP).empty())
    {
      return -1;
    }
    return integerProperties.at(SMTK_TESS_GEN_PROP)[0];
  }

  return -1;
}

/**\brief Set the tessellation generation number.
  *
  * This method is intended for use by mesh resources.
  * Other resources should use resetTessellation() or setTessellation()
  * to automatically advance the generation number.
  *
  * Returns true when the generation number is set and
  * false otherwise (invalid component or out-of-order generation number).
  */
bool EntityRef::setTessellationGeneration(int gen)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& integerProperties = comp->properties().get<std::vector<long>>();
    if (
      integerProperties.contains(SMTK_TESS_GEN_PROP) &&
      !integerProperties.at(SMTK_TESS_GEN_PROP).empty() &&
      integerProperties.at(SMTK_TESS_GEN_PROP)[0] >= gen)
    {
      return false;
    }
    comp->properties().get<std::vector<long>>()[SMTK_TESS_GEN_PROP] = std::vector<long>(1, gen);
    return true;
  }

  return false;
}

/**\brief Set the bounding box of the entity.
  *
  * This should not be called outside of an operator.
  */
void EntityRef::setBoundingBox(const double bbox[6])
{
  std::vector<double> coords(bbox, bbox + 6);
  m_resource.lock()->setBoundingBox(m_entity, coords, 1);
}

/** @name Attribute associations
  *
  */
///@{
/**\brief Does the entityref have any attributes associated with it?
  * Note that this method uses the resource manager.  If the corresponding
  * model resource is unmanaged this will return false.
  */
bool EntityRef::hasAttributes() const
{
  auto comp = this->component();
  if (comp == nullptr)
  {
    return false;
  }
  auto objs = comp->links().linkedFrom(smtk::attribute::Resource::AssociationRole);
  return std::any_of(objs.begin(), objs.end(), [](const smtk::resource::PersistentObjectPtr& obj) {
    // If this is an attribute then return true
    return dynamic_pointer_cast<smtk::attribute::Attribute>(obj) != nullptr;
  });
}

/** @name Attribute associations
  *
  */
///@{
/**\brief Does the entityref have any attributes associated with it from a specific attribute resource?
  */
bool EntityRef::hasAttributes(smtk::attribute::ConstResourcePtr attRes) const
{
  auto comp = this->component();
  if (comp == nullptr)
  {
    return false;
  }
  return attRes->hasAttributes(comp);
}

/**\brief Does the entityref have a specific attribute (represented by an UUID) associate to it.
 * Note that this method requires a resource manager.  If the model and/or attribute resource is
 * not managed, the method will always return false  - To be deprecated
 */
bool EntityRef::hasAttribute(const smtk::common::UUID& attribId) const
{
  auto comp = this->component();
  if (comp == nullptr)
  {
    return false;
  }
  auto objs = comp->links().linkedFrom(smtk::attribute::Resource::AssociationRole);
  return std::any_of(
    objs.begin(), objs.end(), [&attribId](const smtk::resource::PersistentObjectPtr& obj) {
      auto att = dynamic_pointer_cast<smtk::attribute::Attribute>(obj);
      // If this is an attribute then see if it matches the UUID
      return att && att->id() == attribId;
    });
}

/**\brief Does the entityref have any attributes associated with it? - To be deprecated
  */
bool EntityRef::associateAttribute(
  smtk::attribute::ResourcePtr attResource,
  const smtk::common::UUID& attribId)
{
  auto comp = this->component();
  auto att = attResource->findAttribute(attribId);
  if ((comp == nullptr) || (att == nullptr))
  {
    return false;
  }
  return att->associate(comp);
}

/**\brief Disassociate an attribute from the entity? - To be deprecated
  */
bool EntityRef::disassociateAttribute(
  smtk::attribute::ResourcePtr attResource,
  const smtk::common::UUID& attribId)
{
  auto att = attResource->findAttribute(attribId);
  auto comp = this->component();
  if ((att == nullptr) || (comp == nullptr))
  {
    return true; // there is no attribute to disassociate
  }
  return att->disassociate(comp);
}

/**\brief Disassociate an attribute from the entity? - To be deprecated!
  */
bool EntityRef::disassociateAttribute(
  smtk::attribute::ResourcePtr attResource,
  const smtk::common::UUID& attribId,
  bool /*unused*/)
{
  return this->disassociateAttribute(attResource, attribId);
}

/**\brief Remove all attribute association form this entityref
  */
bool EntityRef::disassociateAllAttributes(smtk::attribute::ResourcePtr attResource)
{
  auto comp = this->component();
  if (comp == nullptr)
  {
    return false;
  }
  attResource->disassociateAllAttributes(comp);
  return true;
}

/**\brief Remove all attribute association form this entityref - To be deprecated!
  */
bool EntityRef::disassociateAllAttributes(smtk::attribute::ResourcePtr attResource, bool /*unused*/)
{
  return this->disassociateAllAttributes(attResource);
}

/**\brief Return the attributes associated with the entity
that are of type (or derived type) def.
  */
smtk::attribute::Attributes EntityRef::attributes(smtk::attribute::ConstDefinitionPtr def) const
{
  auto ent = this->entityRecord();
  if (ent == nullptr)
  {
    smtk::attribute::Attributes foo;
    return foo;
  }
  return ent->attributes(def);
}

///@}

/**\brief Return the attributes associated with the entity
that are from a specific attribute resource.
  */
smtk::attribute::Attributes EntityRef::attributes(smtk::attribute::ConstResourcePtr res) const
{
  auto comp = this->component();
  smtk::attribute::Attributes result;
  if (comp == nullptr)
  {
    return result;
  }
  auto atts = res->attributes(comp);
  result.insert(result.begin(), atts.begin(), atts.end());
  return result;
}

///@}

/**\brief Return the attributes associated with the entity
that are from attribute resources managed by the resource manager
associated to the model resource.
  */
smtk::attribute::Attributes EntityRef::attributes() const
{
  auto comp = this->component();
  smtk::attribute::Attributes atts;
  if (comp == nullptr)
  {
    return atts;
  }
  auto objs = comp->links().linkedFrom(smtk::attribute::Resource::AssociationRole);
  for (const auto& obj : objs)
  {
    auto att = std::dynamic_pointer_cast<smtk::attribute::Attribute>(obj);
    // If this is an attribute resource, get the approproate attributes
    if (att != nullptr)
    {
      atts.push_back(att);
    }
  }
  return atts;
}

///@}

void EntityRef::setFloatProperty(const std::string& propName, smtk::model::Float propValue)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    comp->properties().get<std::vector<double>>()[propName] = { propValue };
  }
}

void EntityRef::setFloatProperty(
  const std::string& propName,
  const smtk::model::FloatList& propValue)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    comp->properties().get<std::vector<double>>()[propName] = propValue;
  }
}

smtk::model::FloatList const& EntityRef::floatProperty(const std::string& propName) const
{
  static smtk::model::FloatList dummy;
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& floatProperties = comp->properties().get<std::vector<double>>();
    if (floatProperties.contains(propName))
    {
      return floatProperties.at(propName);
    }
  }
  return dummy;
}

smtk::model::FloatList& EntityRef::floatProperty(const std::string& propName)
{
  static smtk::model::FloatList dummy;
  auto comp = this->component();
  if (comp != nullptr)
  {
    auto floatProperties = comp->properties().get<std::vector<double>>();
    return floatProperties[propName];
  }
  return dummy;
}

bool EntityRef::hasFloatProperty(const std::string& propName) const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& floatProperties = comp->properties().get<std::vector<double>>();
    return floatProperties.contains(propName);
  }
  return false;
}

bool EntityRef::removeFloatProperty(const std::string& propName)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    auto floatProperties = comp->properties().get<std::vector<double>>();
    if (floatProperties.contains(propName))
    {
      floatProperties.erase(propName);
      return true;
    }
  }
  return false;
}

/**\brief Does this entity have *any* float-valued properties?
  *
  * If not, you should call EntityRef::setFloatProperty() before
  * calling EntityRef::floatProperties().
  */
bool EntityRef::hasFloatProperties() const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& floatProperties = comp->properties().get<std::vector<double>>();
    return !floatProperties.empty();
  }
  return false;
}

/// Return the names of all the floating-point properties.
std::set<std::string> EntityRef::floatPropertyNames() const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& floatProperties = comp->properties().get<std::vector<double>>();
    return floatProperties.keys();
  }
  return std::set<std::string>();
}

void EntityRef::setStringProperty(const std::string& propName, const smtk::model::String& propValue)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    comp->properties().get<std::vector<std::string>>()[propName] = { propValue };
  }
}

void EntityRef::setStringProperty(
  const std::string& propName,
  const smtk::model::StringList& propValue)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    comp->properties().get<std::vector<std::string>>()[propName] = propValue;
  }
}

smtk::model::StringList const& EntityRef::stringProperty(const std::string& propName) const
{
  static smtk::model::StringList dummy;
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& stringProperties = comp->properties().get<std::vector<std::string>>();
    if (stringProperties.contains(propName))
    {
      return stringProperties.at(propName);
    }
  }
  return dummy;
}

smtk::model::StringList& EntityRef::stringProperty(const std::string& propName)
{
  static smtk::model::StringList dummy;
  auto comp = this->component();
  if (comp != nullptr)
  {
    auto stringProperties = comp->properties().get<std::vector<std::string>>();
    return stringProperties[propName];
  }
  return dummy;
}

bool EntityRef::hasStringProperty(const std::string& propName) const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& stringProperties = comp->properties().get<std::vector<std::string>>();
    return stringProperties.contains(propName);
  }
  return false;
}

bool EntityRef::removeStringProperty(const std::string& propName)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    auto stringProperties = comp->properties().get<std::vector<std::string>>();
    if (stringProperties.contains(propName))
    {
      stringProperties.erase(propName);
      return true;
    }
  }
  return false;
}

/**\brief Does this entity have *any* string-valued properties?
  *
  * If not, you should call EntityRef::setStringProperty() before
  * calling EntityRef::stringProperties().
  */
bool EntityRef::hasStringProperties() const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& stringProperties = comp->properties().get<std::vector<std::string>>();
    return !stringProperties.empty();
  }
  return false;
}

/// Return the names of all the string properties.
std::set<std::string> EntityRef::stringPropertyNames() const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& stringProperties = comp->properties().get<std::vector<std::string>>();
    return stringProperties.keys();
  }
  return std::set<std::string>();
}

void EntityRef::setIntegerProperty(const std::string& propName, smtk::model::Integer propValue)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    comp->properties().get<std::vector<long>>()[propName] = { propValue };
  }
}

void EntityRef::setIntegerProperty(
  const std::string& propName,
  const smtk::model::IntegerList& propValue)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    comp->properties().get<std::vector<long>>()[propName] = propValue;
  }
}

smtk::model::IntegerList const& EntityRef::integerProperty(const std::string& propName) const
{
  static smtk::model::IntegerList dummy;
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& integerProperties = comp->properties().get<std::vector<long>>();
    if (integerProperties.contains(propName))
    {
      return integerProperties.at(propName);
    }
  }
  return dummy;
}

smtk::model::IntegerList& EntityRef::integerProperty(const std::string& propName)
{
  static smtk::model::IntegerList dummy;
  auto comp = this->component();
  if (comp != nullptr)
  {
    auto integerProperties = comp->properties().get<std::vector<long>>();
    return integerProperties[propName];
  }
  return dummy;
}

bool EntityRef::hasIntegerProperty(const std::string& propName) const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& integerProperties = comp->properties().get<std::vector<long>>();
    return integerProperties.contains(propName);
  }
  return false;
}

bool EntityRef::removeIntegerProperty(const std::string& propName)
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    auto integerProperties = comp->properties().get<std::vector<long>>();
    if (integerProperties.contains(propName))
    {
      integerProperties.erase(propName);
      return true;
    }
  }
  return false;
}

/**\brief Does this entity have *any* integer-valued properties?
  *
  * If not, you should call EntityRef::setIntegerProperty() before
  * calling EntityRef::integerProperties().
  */
bool EntityRef::hasIntegerProperties() const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& integerProperties = comp->properties().get<std::vector<long>>();
    return !integerProperties.empty();
  }
  return false;
}

/// Return the names of all the integer properties.
std::set<std::string> EntityRef::integerPropertyNames() const
{
  auto comp = this->component();
  if (comp != nullptr)
  {
    const auto& integerProperties = comp->properties().get<std::vector<long>>();
    return integerProperties.keys();
  }
  return std::set<std::string>();
}

/// Return the number of arrangements of the given kind \a k.
int EntityRef::numberOfArrangementsOfKind(ArrangementKind k) const
{
  ResourcePtr rsrc = m_resource.lock();
  const Arrangements* arr = rsrc->hasArrangementsOfKindForEntity(m_entity, k);
  return arr ? static_cast<int>(arr->size()) : 0;
}

/// Return the \a i-th arrangement of kind \a k (or nullptr).
Arrangement* EntityRef::findArrangement(ArrangementKind k, int i)
{
  ResourcePtr rsrc = m_resource.lock();
  return rsrc->findArrangement(m_entity, k, i);
}

/// Return the \a i-th arrangement of kind \a k (or nullptr).
const Arrangement* EntityRef::findArrangement(ArrangementKind k, int i) const
{
  ResourcePtr rsrc = m_resource.lock();
  return rsrc->findArrangement(m_entity, k, i);
}

/// Delete all arrangements of this entity with prejudice.
bool EntityRef::clearArrangements()
{
  ResourcePtr rsrc = m_resource.lock();
  return rsrc->clearArrangements(m_entity);
}

/**\brief Return the relation specified by the \a offset into the specified arrangement.
  *
  */
EntityRef EntityRef::relationFromArrangement(ArrangementKind k, int arrangementIndex, int offset)
  const
{
  ResourcePtr rsrc = m_resource.lock();
  EntityPtr ent = rsrc->findEntity(m_entity);
  if (ent)
  {
    const Arrangement* arr = this->findArrangement(k, arrangementIndex);
    if (arr && static_cast<int>(arr->details().size()) > offset)
    {
      int idx = arr->details()[offset];
      return idx < 0 ? EntityRef() : EntityRef(rsrc, ent->relations()[idx]);
    }
  }
  return EntityRef();
}

/**\brief Remove an arrangement.
  *
  * This will remove an arrangement -- starting with the side held by this entity.
  * Unlike other methods that generally start with the "parent" (the "one" in a
  * "one-to-many" relationship), this starts with the current entity (even if it
  * is the "child" or one of the "many").
  * This makes it possible to remove a broken, one-sided arrangement.
  *
  * If \a index is invalid (i.e., negative), then the first arrangement of the
  * given kind will be removed.
  */
bool EntityRef::removeArrangement(ArrangementKind k, int index)
{
  ResourcePtr rsrc = m_resource.lock();
  return rsrc ? rsrc->unarrangeEntity(m_entity, k, index < 0 ? 0 : index) != 0 : false;
}

/**\brief Embed the specified \a thingToEmbed as an inclusion into this entityref's entity.
  *
  * This adds an INCLUDES relation (if necessary) to this entity and
  * an EMBEDDED_IN relation (if necessary) to the \a thingToEmbed.
  *
  * If \a checkExistence is true, then the relationship is not added if it would be
  * a duplicate. This check is expensive because relationships are not indexed, so
  * if you know \a thingToEmbed is not already embedded, pass false to \a checkExistence.
  * The default is true.
  */
EntityRef& EntityRef::embedEntity(const EntityRef& thingToEmbed, bool checkExistence)
{
  ResourcePtr rsrc = m_resource.lock();
  //ResourceEventType event = std::make_pair(ADD_EVENT, INVALID_RELATIONSHIP);
  ResourceEventType event = std::make_pair(ADD_EVENT, this->embeddingRelationType(thingToEmbed));
  if (event.second != INVALID_RELATIONSHIP)
  {
    if (checkExistence)
    {
      EntityRefArrangementOps::findOrAddSimpleRelationship(*this, INCLUDES, thingToEmbed);
      EntityRefArrangementOps::findOrAddSimpleRelationship(thingToEmbed, EMBEDDED_IN, *this);
    }
    else
    {
      EntityRefArrangementOps::addSimpleRelationship(*this, INCLUDES, thingToEmbed, checkExistence);
      EntityRefArrangementOps::addSimpleRelationship(
        thingToEmbed, EMBEDDED_IN, *this, checkExistence);
    }
    rsrc->trigger(event, *this, thingToEmbed);
  }
  return *this;
}

/**\brief Unembed the specified \a thingToUnembed as an inclusion into this entityref's entity.
  *
  * This removes an INCLUDES relation (if necessary) to this entity and
  * an EMBEDDED_IN relation (if necessary) to the \a thingToUnembed.
  */
bool EntityRef::unembedEntity(const EntityRef& thingToEmbed)
{
  ResourcePtr rsrc = m_resource.lock();
  ResourceEventType event = std::make_pair(DEL_EVENT, this->embeddingRelationType(thingToEmbed));
  if (event.second != INVALID_RELATIONSHIP)
  {
    int aidx = EntityRefArrangementOps::findSimpleRelationship(*this, INCLUDES, thingToEmbed);
    if (aidx >= 0)
    {
      rsrc->unarrangeEntity(m_entity, INCLUDES, aidx);
      rsrc->trigger(event, *this, thingToEmbed);
      return true;
    }
  }
  return false;
}

/**\brief Return whether the specified \a entity is a direct inclusion in this entityref's entity.
  *
  */
bool EntityRef::isEmbedded(EntityRef& ent) const
{
  return EntityRefArrangementOps::findSimpleRelationship(*this, INCLUDES, ent) >= 0;
}

/**\brief Report the entity into which this entity is directly embedded.
  *
  * If the entity is not embedded into another, the result will be an invalid entityref.
  */
EntityRef EntityRef::embeddedIn() const
{
  return EntityRefArrangementOps::firstRelation<EntityRef>(*this, EMBEDDED_IN);
}

/**\brief Return the Model which owns this entity.
  *
  * A Model is analogous to a body in some kernels,
  * a lump in others, or a group of regions in yet others.
  * In SMTK, models are the objects that get associated with
  * sessions to other kernels, so determining the owning
  * model is important for discovering which modeling operations
  * (including reading and writing to/from disk) are possible.
  *
  * Note that the returned model may be invalid.
  */
Model EntityRef::owningModel() const
{
  ResourcePtr rsrc = m_resource.lock();
  return rsrc ? Model(rsrc, rsrc->modelOwningEntity(m_entity)) : Model();
}

/**\brief Return the SessionRef which owns this entity.
  *
  * A session is a collection of models held at the same URL
  * (i.e., in the same file), if loaded from storage, and
  * managed by the same modeling kernel.
  */
SessionRef EntityRef::owningSession() const
{
  ResourcePtr rsrc = m_resource.lock();
  return rsrc ? SessionRef(rsrc, rsrc->sessionOwningEntity(m_entity)) : SessionRef();
}

/**\brief Return the Groups which contains this entity.
  *
  */
Groups EntityRef::containingGroups() const
{
  Groups result;
  EntityRefArrangementOps::appendAllRelations(*this, SUBSET_OF, result);
  return result;
}

/**\brief Return true when this EntityRef has a SUPERSET_OF relationship with \a ent.
  */
bool EntityRef::isMember(const EntityRef& ent) const
{
  return EntityRefArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, ent) >= 0;
}

/**\brief Return the first entity that this EntityRef has a SUBSET_OF relationship with.
  */
EntityRef EntityRef::memberOf() const
{
  return EntityRefArrangementOps::firstRelation<EntityRef>(*this, SUBSET_OF);
}

/// A comparator provided so that entityrefs may be included in ordered sets.
bool EntityRef::operator==(const EntityRef& other) const
{
  ResourcePtr rsrc = m_resource.lock();
  return (rsrc == other.resource()) && (m_entity == other.m_entity);
}

/// A comparator provided for convenience.
bool EntityRef::operator!=(const EntityRef& other) const
{
  return !(*this == other);
}

/// A comparator provided so that entityrefs may be included in ordered sets.
bool EntityRef::operator<(const EntityRef& other) const
{
  ResourcePtr rsrc = m_resource.lock();
  ResourcePtr otherMgr = other.m_resource.lock();
  if (rsrc < otherMgr)
  {
    return true;
  }
  else if (otherMgr < rsrc)
  {
    return false;
  }
  return m_entity < other.m_entity;
}

/**\brief A hash function for entityrefs.
  *
  * This allows entityrefs to be put into sets in Python.
  */
std::size_t EntityRef::hash() const
{
  ResourcePtr rsrc = m_resource.lock();
  std::size_t result = m_entity.hash();
  boost::hash_combine(result, reinterpret_cast<std::size_t>(rsrc.get()));
  return result;
}

std::ostream& operator<<(std::ostream& os, const EntityRef& c)
{
  os << c.name();
  return os;
}

std::size_t entityrefHash(const EntityRef& c)
{
  return c.hash();
}

/*! \fn template<typename T> T EntityRef::relationsAs() const
 *\brief Return all of the entities related to this entityref.
 *
 * The return value is a template parameter naming container type.
 * Each member is cast from a EntityRef to T::value_type and added
 * to the container only if valid. This makes it possible to
 * subset the relations by forcing them into a container of the
 * desired type.
 */

/*! \fn template<typename S, typename T> void EntityRef::EntityRefsFromUUIDs(S& result, ResourcePtr rsrc, const T& uids)
 *\brief Convert a set of UUIDs into a set of entityrefs referencing the same \a rsrc.
 *
 * Only valid entities are inserted into \a result.
 * This means that the UUID must be non-nullptr **and** have a corresponding
 * entry in the model resource, \a rsrc, that matches the output container type
 * in order to appear in \a result.
 */

/*! \fn template<typename S, typename T> void EntityRef::EntityRefsFromUUIDs(S& result, const T& entRefs)
 *\brief Convert a set of entity references into just UUIDs.
 *
 * EntityRef entities in \a entRefs are **not** checked for validity before insertion;
 * however, only non-nullptr UUIDs are inserted into \a result.
 * (This means that cursors that do not have entries in the model resource will
 * still have their UUIDs added to the result.)
 */

/*! \fn template<typename T> EntityRef::embedEntities(const T& container)
 * \brief Embed each of the entities in the container inside this entity.
 */

/*! \fn EntityRef::instances() const
 * \brief Return all the instances this object serves as a prototype for.
 */

/*! \fn EntityRef::removeProperty<T>(const std::string& name)
 *  \brief Remove the property of type \a T with the given \a name, returning true on success.
 *
 * False is returned if the property did not exist for the given entity.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template<>
SMTKCORE_EXPORT bool EntityRef::removeProperty<StringData>(const std::string& pname)
{
  return this->removeStringProperty(pname);
}

template<>
SMTKCORE_EXPORT bool EntityRef::removeProperty<FloatData>(const std::string& pname)
{
  return this->removeFloatProperty(pname);
}

template<>
SMTKCORE_EXPORT bool EntityRef::removeProperty<IntegerData>(const std::string& pname)
{
  return this->removeIntegerProperty(pname);
}

/**\defgroup SetMembershipMethods Methods for managing superset/subset membership.
  *\brief Primitive operations for subset/superset arrangements.
  *
  * These methods are intended to be used by subclasses such as Group and Model.
  */
///@{
/**\brief Add an entity as a member of this entity without any membership constraint checks.
  *
  */
EntityRef& EntityRef::addMemberEntity(const EntityRef& memberToAdd)
{
  ResourcePtr rsrc = m_resource.lock();
  ResourceEventType event = std::make_pair(ADD_EVENT, this->subsetRelationType(memberToAdd));
  if (event.second != INVALID_RELATIONSHIP)
  {
    EntityRefArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, memberToAdd);
    EntityRefArrangementOps::findOrAddSimpleRelationship(memberToAdd, SUBSET_OF, *this);
    rsrc->trigger(event, *this, memberToAdd);
  }
  return *this;
}

/**\brief Remove the \a memberToRemove from this EntityRef.
  *
  * No membership constraint checks are performed.
  */
EntityRef& EntityRef::removeMemberEntity(const EntityRef& memberToRemove)
{
  int aidx = EntityRefArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, memberToRemove);
  return this->removeMemberEntity(aidx);
}

/**\brief Remove a member from this EntityRef given the index of its arrangement.
  *
  * No membership constraint checks are performed.
  *
  * Note that the \a indexOfArrangementToRemove is not an index into the Entity's
  * relations, but an index into the Arrangement vector for the SUPERSET_OF relationship.
  */
EntityRef& EntityRef::removeMemberEntity(int indexOfArrangementToRemove)
{
  if (indexOfArrangementToRemove < 0)
    return *this;

  ResourcePtr rsrc = m_resource.lock();
  EntityRef memberToRemove =
    this->relationFromArrangement(SUPERSET_OF, indexOfArrangementToRemove, 0);
  ResourceEventType event = std::make_pair(DEL_EVENT, this->embeddingRelationType(memberToRemove));
  if (event.second != INVALID_RELATIONSHIP)
  {
    rsrc->unarrangeEntity(m_entity, SUPERSET_OF, indexOfArrangementToRemove);
    rsrc->trigger(event, *this, memberToRemove);
  }
  return *this;
}

/**\brief Determine the nature of a SUPERSET_OF relationship.
  *
  * Given a SUPERSET_OF arrangement, use the types of the involved
  * entities to determine the exact nature of the relationship.
  * This is used by the event framework so that observers can watch
  * for particular types of events.
  */
ResourceEventRelationType EntityRef::subsetRelationType(const EntityRef& member) const
{
  ResourceEventRelationType reln = INVALID_RELATIONSHIP;

  switch (this->entityFlags() & ENTITY_MASK)
  {
    case SESSION:
      switch (member.entityFlags() & ENTITY_MASK)
      {
        case MODEL_ENTITY:
          reln = SESSION_SUPERSET_OF_MODEL;
          break;
      }
      break;
    case MODEL_ENTITY:
      switch (member.entityFlags() & ENTITY_MASK)
      {
        case MODEL_ENTITY:
          reln = MODEL_SUPERSET_OF_MODEL;
          break;
        case GROUP_ENTITY:
          reln = MODEL_SUPERSET_OF_GROUP;
          break;
      }
      break;
    case GROUP_ENTITY:
      reln = GROUP_SUPERSET_OF_ENTITY;
      break;
    default:
      break;
  }

  return reln;
}

///@}

/**\brief Determine the nature of an EMBEDDED_IN relationship.
  *\ingroup embeddingMethods
  *
  * Given an EMBEDDED_IN arrangement, use the types of the involved
  * entities to determine the exact nature of the relationship.
  * This is used by the event framework so that observers can watch
  * for particular types of events.
  */
ResourceEventRelationType EntityRef::embeddingRelationType(const EntityRef& embedded) const
{
  ResourceEventRelationType reln = INVALID_RELATIONSHIP;

  switch (this->entityFlags() & ENTITY_MASK)
  {
    case SESSION:
      switch (embedded.entityFlags() & ENTITY_MASK)
      {
        case MODEL_ENTITY:
          reln = SESSION_INCLUDES_MODEL;
          break;
      }
      break;
    case MODEL_ENTITY:
      switch (embedded.entityFlags() & ENTITY_MASK)
      {
        case MODEL_ENTITY:
          reln = MODEL_INCLUDES_MODEL;
          break;
        case SHELL_ENTITY:
          reln = MODEL_INCLUDES_FREE_SHELL;
          break;
        case CELL_ENTITY:
          reln = MODEL_INCLUDES_FREE_CELL;
          break;
        case AUX_GEOM_ENTITY:
          reln = MODEL_INCLUDES_FREE_AUX_GEOM;
          break;
        case USE_ENTITY:
          reln = MODEL_INCLUDES_FREE_USE;
          break;
      }
      break;
    case CELL_ENTITY:
      switch (embedded.entityFlags() & ENTITY_MASK)
      {
        case CELL_ENTITY:
          reln = CELL_INCLUDES_CELL;
          break;
      }
      break;
    case SHELL_ENTITY:
      switch (embedded.entityFlags() & ENTITY_MASK)
      {
        case SHELL_ENTITY:
          reln = SHELL_INCLUDES_SHELL;
          break;
        case USE_ENTITY:
          reln = SHELL_HAS_USE;
          break;
      }
      break;
    case AUX_GEOM_ENTITY:
      switch (embedded.entityFlags() & ENTITY_MASK)
      {
        case AUX_GEOM_ENTITY:
          reln = AUX_GEOM_INCLUDES_AUX_GEOM;
          break;
      }
      break;
  }

  return reln;
}

/**\brief A convenient method for finding all entities with tessellations owned by this entity.
  *
  * Note that the \a entityrefMap is a map of entity with tessellation to its parent, and
  * the \a touched is a set of entities already visited.
  *
  */
void EntityRef::findEntitiesWithTessellation(
  std::map<smtk::model::EntityRef, smtk::model::EntityRef>& entityrefMap,
  std::set<smtk::model::EntityRef>& touched) const
{
  EntityRefArray children =
    (this->isModel()
       ? this->as<Model>().cellsAs<EntityRefArray>()
       : (this->isCellEntity()
            ? this->as<CellEntity>().boundingCellsAs<EntityRefArray>()
            : (this->isGroup() ? this->as<Group>().members<EntityRefArray>() : EntityRefArray())));
  if (this->isModel())
  {
    // Make sure groups are handled last to avoid unexpected "parents" in entityrefMap.
    EntityRefArray tmp;
    tmp = this->as<Model>().submodelsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
    tmp = this->as<Model>().groupsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
  }
  for (EntityRefArray::const_iterator it = children.begin(); it != children.end(); ++it)
  {
    if (touched.find(*it) == touched.end())
    {
      touched.insert(*it);
      if (it->hasTessellation())
      {
        entityrefMap[*it] = *this;
      }
      it->findEntitiesWithTessellation(entityrefMap, touched);
    }
  }
}

} // namespace model
} // namespace smtk

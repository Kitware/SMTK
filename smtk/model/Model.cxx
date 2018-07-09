//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Model.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

namespace smtk
{
namespace model
{

/**\brief Return information about how the modeling kernel represents this model.
  *
  * \sa ModelGeometryStyle
  */
ModelGeometryStyle Model::geometryStyle() const
{
  if (this->hasIntegerProperty(SMTK_GEOM_STYLE_PROP))
  {
    const smtk::model::IntegerList& plist(this->integerProperty(SMTK_GEOM_STYLE_PROP));
    if (!plist.empty())
      return static_cast<ModelGeometryStyle>(plist[0]);
  }
  return PARAMETRIC;
}

/**\brief Set the number coordinate values used to specify each point in the locus of this model.
  *
  * This should meet or exceed the maxParametricDimension() of every entity in the model.
  *
  * WARNING: This is not intended to be changed during the lifetime of a model;
  *          this method exists for the convenience of session classes.
  */
void Model::setEmbeddingDimension(int dim)
{
  this->setIntegerProperty("embedding dimension", dim);
}

/// Return the parent of this entity, which should be a SessionRef or another Model.
EntityRef Model::parent() const
{
  return EntityRefArrangementOps::firstRelation<EntityRef>(*this, SUBSET_OF);
}

/**\brief Return a reference to the session that owns this model.
  *
  * Note that a model may be owned by other models, so
  * this differs from Model::parent() in that it continues up
  * the tree of parent models until it finds an owning Session.
  *
  * If a model is ill-defined (invalid or with an invalid session)
  * the returned SessionRef will be invalid.
  *
  * \sa SessionRef
  */
SessionRef Model::session() const
{
  return this->owningSession();
}

/// Update this model so that it refers to \a sess as its owner.
void Model::setSession(const SessionRef& sess)
{
  SessionRef curSess = this->session();
  if (curSess == sess)
    return;
  if (curSess.isValid())
    curSess.removeMemberEntity(*this);

  if (sess.isValid())
  {
    SessionRef mutableSess(sess);
    mutableSess.addModel(*this);
    //mutableSess.addMemberEntity(*this);
  }
}

bool Model::isModified() const
{
  smtk::model::Resource::Ptr resource = this->resource();
  if (!resource || !this->hasIntegerProperty("clean"))
  {
    return false;
  }

  const IntegerList& prop(this->integerProperty("clean"));
  return (!prop.empty() && (prop[0] != 1));
}

void Model::setIsModified(bool isModified)
{
  smtk::model::Resource::Ptr resource = this->resource();
  if (!resource)
  {
    return;
  }

  this->setIntegerProperty("clean", isModified ? 0 : 1);
}

/// Return the cells directly owned by this model.
CellEntities Model::cells() const
{
  CellEntities result;
  ResourcePtr resource = this->resource();
  if (!resource)
  {
    return result;
  }
  EntityRefArrangementOps::appendAllRelations(*this, INCLUDES, result);
  if (result.empty())
  { // We may have a "simple" model that has no arrangements but does have relations.
    EntityPtr erec = resource->findEntity(m_entity);
    if (erec)
    {
      smtk::common::UUIDArray::const_iterator rit;
      for (rit = erec->relations().begin(); rit != erec->relations().end(); ++rit)
      {
        CellEntity cell(resource, *rit);
        if (cell.isValid())
          result.push_back(cell);
      }
    }
  }
  return result;
}

/// Return the groups directly owned by this model.
Groups Model::groups() const
{
  Groups result;
  EntityRefArrangementOps::appendAllRelations(*this, SUPERSET_OF, result);
  return result;
}

/// Return the models directly owned by this model.
Models Model::submodels() const
{
  Models result;
  EntityRefArrangementOps::appendAllRelations(*this, SUPERSET_OF, result);
  return result;
}

/// Return the auxiliary geometry instances directly owned by this model.
AuxiliaryGeometries Model::auxiliaryGeometry() const
{
  AuxiliaryGeometries result;
  EntityRefArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

Model& Model::addCell(const CellEntity& c)
{
  this->embedEntity(c);
  return *this;
}

Model& Model::removeCell(const CellEntity& c)
{
  ResourcePtr resource = this->resource();
  bool ok = this->unembedEntity(c);
  (void)ok;
  /*
  if (!ok)
    {
    std::cout << "Unable to remove cell " << c.name() << " from model\n";
    }
    */
  UUIDWithEntityPtr ent;
  ent = resource->topology().find(m_entity);
  if (ent != resource->topology().end())
  {
    resource->elideOneEntityReference(ent, c.entity());
  }
  ent = resource->topology().find(c.entity());
  if (ent != resource->topology().end())
  {
    resource->elideOneEntityReference(ent, m_entity);
  }
  return *this;
}

Model& Model::addGroup(const Group& g)
{
  ResourcePtr resource = this->resource();
  if (this->isValid() && g.isValid())
  {
    EntityRefArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, g);
    EntityRefArrangementOps::findOrAddSimpleRelationship(g, SUBSET_OF, *this);
    resource->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_GROUP), *this, g);
  }
  return *this;
}

Model& Model::removeGroup(const Group& g)
{
  ResourcePtr resource = this->resource();
  if (this->isValid() && g.isValid())
  {
    int aidx = EntityRefArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, g);
    if (aidx >= 0 && resource->unarrangeEntity(m_entity, SUPERSET_OF, aidx) > 0)
    {
      resource->trigger(ResourceEventType(DEL_EVENT, MODEL_INCLUDES_GROUP), *this, g);
    }
  }
  return *this;
}

Model& Model::addSubmodel(const Model& m)
{
  this->addMemberEntity(m);
  return *this;
  /*
  ResourcePtr resource = this->resource();
  if (this->isValid() && m.isValid() && m.resource() == this->resource() && m.entity() != this->entity())
    {
    EntityRefArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, m);
    EntityRefArrangementOps::findOrAddSimpleRelationship(m, SUBSET_OF, *this);
    resource->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_MODEL), *this, m);
    }
  return *this;
  */
}

Model& Model::removeSubmodel(const Model& m)
{
  this->removeMemberEntity(m);
  return *this;
  /*
  ResourcePtr resource = this->resource();
  if (this->isValid() && m.isValid() && m.resource() == this->resource() && m.entity() != this->entity())
    {
    int aidx = EntityRefArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, m);
    if (aidx >= 0 && resource->unarrangeEntity(m_entity, SUPERSET_OF, aidx) > 0)
      {
      resource->trigger(ResourceEventType(DEL_EVENT, MODEL_INCLUDES_MODEL), *this, m);
      }
    }
  return *this;
  */
}

/// Add the given auxiliary geometry to this model.
Model& Model::addAuxiliaryGeometry(const AuxiliaryGeometry& ag)
{
  this->embedEntity(ag);
  return *this;
}

/**\brief Remove the given auxiliary geometry from this model (if present as a top-level entry).
  *
  *
  */
Model& Model::removeAuxiliaryGeometry(const AuxiliaryGeometry& ag)
{
  this->unembedEntity(ag);
  return *this;
}

/// An efficient method for assigning default names to all of the model's entities.
void Model::assignDefaultNames()
{
  this->resource()->assignDefaultNamesToModelChildren(this->entity());
}

/**\brief A convenient method for finding all entities with tessellations owned by this model.
  *
  * Note that this method will include both cells and groups that have tessellations.
  *
  */
EntityRefs Model::entitiesWithTessellation() const
{
  std::map<EntityRef, EntityRef> entityrefMap;
  EntityRefs touched;
  this->findEntitiesWithTessellation(entityrefMap, touched);
  EntityRefs result;
  for (std::map<EntityRef, EntityRef>::iterator it = entityrefMap.begin(); it != entityrefMap.end();
       ++it)
  {
    result.insert(it->first);
  }
  return result;
}

} // namespace model
} // namespace smtk

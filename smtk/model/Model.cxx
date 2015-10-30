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

#include "smtk/model/Session.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

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
  std::set<EntityRef> parents; // prevent infinite loops
  EntityRef pp = this->parent();
  while (pp.isValid() && !pp.isSessionRef() && parents.find(pp) == parents.end())
    pp = pp.as<Model>().parent();

  return pp.as<SessionRef>();
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
    mutableSess.addMemberEntity(*this);
    }
}

/// Return the cells directly owned by this model.
CellEntities Model::cells() const
{
  CellEntities result;
  ManagerPtr mgr = this->manager();
  if (!mgr)
    {
    return result;
    }
  EntityRefArrangementOps::appendAllRelations(*this, INCLUDES, result);
  if (result.empty())
    { // We may have a "simple" model that has no arrangements but does have relations.
    const Entity* erec = mgr->findEntity(this->m_entity);
    if (erec)
      {
      smtk::common::UUIDArray::const_iterator rit;
      for (rit = erec->relations().begin(); rit != erec->relations().end(); ++rit)
        {
        CellEntity cell(mgr, *rit);
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

Model& Model::addCell(const CellEntity& c)
{
  this->embedEntity(c);
  return *this;
}

Model& Model::removeCell(const CellEntity& c)
{
  ManagerPtr mgr = this->manager();
  this->unembedEntity(c);
  UUIDWithEntity ent;
  ent = mgr->topology().find(this->m_entity);
  if (ent != mgr->topology().end())
    {
    mgr->elideOneEntityReference(ent, c.entity());
    }
  ent = mgr->topology().find(c.entity());
  if (ent != mgr->topology().end())
    {
    mgr->elideOneEntityReference(ent, this->m_entity);
    }
  return *this;
}

Model& Model::addGroup(const Group& g)
{
  ManagerPtr mgr = this->manager();
  if (this->isValid() && g.isValid())
    {
    EntityRefArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, g);
    EntityRefArrangementOps::findOrAddSimpleRelationship(g, SUBSET_OF, *this);
    mgr->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_GROUP), *this, g);
    }
  return *this;
}

Model& Model::removeGroup(const Group& g)
{
  ManagerPtr mgr = this->manager();
  if (this->isValid() && g.isValid())
    {
    int aidx = EntityRefArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, g);
    if (aidx >= 0 && mgr->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      {
      mgr->trigger(ManagerEventType(DEL_EVENT, MODEL_INCLUDES_GROUP), *this, g);
      }
    }
  return *this;
}

Model& Model::addSubmodel(const Model& m)
{
  this->addMemberEntity(m);
  return *this;
  /*
  ManagerPtr mgr = this->manager();
  if (this->isValid() && m.isValid() && m.manager() == this->manager() && m.entity() != this->entity())
    {
    EntityRefArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, m);
    EntityRefArrangementOps::findOrAddSimpleRelationship(m, SUBSET_OF, *this);
    mgr->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_MODEL), *this, m);
    }
  return *this;
  */
}

Model& Model::removeSubmodel(const Model& m)
{
  this->removeMemberEntity(m);
  return *this;
  /*
  ManagerPtr mgr = this->manager();
  if (this->isValid() && m.isValid() && m.manager() == this->manager() && m.entity() != this->entity())
    {
    int aidx = EntityRefArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, m);
    if (aidx >= 0 && mgr->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      {
      mgr->trigger(ManagerEventType(DEL_EVENT, MODEL_INCLUDES_MODEL), *this, m);
      }
    }
  return *this;
  */
}

/// Return an operator of the given \a opname with its Manager set to this model's.
OperatorPtr Model::op(const std::string& opname) const
{
  return this->session().op(opname);
}

/*
/// Return a set of the operators available for this model.
Operators Model::operators() const
{
  Operators ops;
  ManagerPtr mgr = this->manager();
  Operators::const_iterator it;
  Session::ConstPtr br = this->session();
  for (
    it = br->operators().begin();
    it != br->operators().end();
    ++it)
    {
    ops.insert((*it)->clone()->setManager(mgr));
    }
  return ops;
}
*/

/// Return the names of all the operators which can be applied to this model.
StringList Model::operatorNames() const
{
  return this->session().operatorNames();
}

/// An efficient method for assigning default names to all of the model's entities.
void Model::assignDefaultNames()
{
  this->manager()->assignDefaultNamesToModelChildren(this->entity());
}

  } // namespace model
} // namespace smtk

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/ModelEntity.h"

#include "smtk/model/Bridge.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

/**\brief Set the number coordinate values used to specify each point in the locus of this model.
  *
  * This should meet or exceed the maxParametricDimension() of every entity in the model.
  *
  * WARNING: This is not intended to be changed during the lifetime of a model;
  *          this method exists for the convenience of bridge classes.
  */
void ModelEntity::setEmbeddingDimension(int dim)
{
  this->setIntegerProperty("embedding dimension", dim);
}

/// Return the parent of this entity, which should be invalid or another ModelEntity.
Cursor ModelEntity::parent() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, EMBEDDED_IN);
}

/// Return the cells directly owned by this model.
CellEntities ModelEntity::cells() const
{
  CellEntities result;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, result);
  if (result.empty())
    { // We may have a "simple" model that has no arrangements but does have relations.
    for (UUIDWithEntity it = this->m_manager->topology().begin(); it != this->m_manager->topology().end(); ++it)
      {
      CellEntity cell(this->m_manager, it->first);
      if (cell.isValid())
        result.push_back(cell);
      }
    }
  return result;
}

/// Return the groups directly owned by this model.
GroupEntities ModelEntity::groups() const
{
  GroupEntities result;
  CursorArrangementOps::appendAllRelations(*this, SUPERSET_OF, result);
  return result;
}

/// Return the models directly owned by this model.
ModelEntities ModelEntity::submodels() const
{
  ModelEntities result;
  CursorArrangementOps::appendAllRelations(*this, SUPERSET_OF, result);
  return result;
}

ModelEntity& ModelEntity::addCell(const CellEntity& c)
{
  this->embedEntity(c);
  return *this;
}

ModelEntity& ModelEntity::removeCell(const CellEntity& c)
{
  this->unembedEntity(c);
  return *this;
}

ModelEntity& ModelEntity::addGroup(const GroupEntity& g)
{
  if (this->isValid() && g.isValid())
    {
    CursorArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, g);
    CursorArrangementOps::findOrAddSimpleRelationship(g, SUBSET_OF, *this);
    this->m_manager->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_GROUP), *this, g);
    }
  return *this;
}

ModelEntity& ModelEntity::removeGroup(const GroupEntity& g)
{
  if (this->isValid() && g.isValid())
    {
    int aidx = CursorArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, g);
    if (aidx >= 0 && this->m_manager->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      {
      this->m_manager->trigger(ManagerEventType(DEL_EVENT, MODEL_INCLUDES_GROUP), *this, g);
      }
    }
  return *this;
}

ModelEntity& ModelEntity::addSubmodel(const ModelEntity& m)
{
  if (this->isValid() && m.isValid() && m.manager() == this->manager() && m.entity() != this->entity())
    {
    CursorArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, m);
    CursorArrangementOps::findOrAddSimpleRelationship(m, SUBSET_OF, *this);
    this->m_manager->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_MODEL), *this, m);
    }
  return *this;
}

ModelEntity& ModelEntity::removeSubmodel(const ModelEntity& m)
{
  if (this->isValid() && m.isValid() && m.manager() == this->manager() && m.entity() != this->entity())
    {
    int aidx = CursorArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, m);
    if (aidx >= 0 && this->m_manager->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      {
      this->m_manager->trigger(ManagerEventType(DEL_EVENT, MODEL_INCLUDES_MODEL), *this, m);
      }
    }
  return *this;
}

/// Return an operator of the given \a opname with its Manager set to this model's.
OperatorPtr ModelEntity::op(const std::string& opname) const
{
  return this->bridge()->op(opname);
}

/*
/// Return a set of the operators available for this model.
Operators ModelEntity::operators() const
{
  Operators ops;
  Operators::const_iterator it;
  Bridge::ConstPtr br = this->bridge();
  for (
    it = br->operators().begin();
    it != br->operators().end();
    ++it)
    {
    ops.insert((*it)->clone()->setManager(this->m_manager));
    }
  return ops;
}
*/

/// Return the names of all the operators which can be applied to this model.
StringList ModelEntity::operatorNames() const
{
  return this->bridge()->operatorNames();
}

BridgePtr ModelEntity::bridge() const
{
  return this->m_manager->bridgeForModel(this->m_entity);
}

  } // namespace model
} // namespace smtk

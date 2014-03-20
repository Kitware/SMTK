#include "smtk/model/ModelEntity.h"

#include "smtk/model/BridgeBase.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

Cursor ModelEntity::parent() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, EMBEDDED_IN);
}

/// Return the cells directly owned by this model.
CellEntities ModelEntity::cells() const
{
  CellEntities result;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, result);
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
    this->m_storage->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_GROUP), *this, g);
    }
  return *this;
}

ModelEntity& ModelEntity::removeGroup(const GroupEntity& g)
{
  if (this->isValid() && g.isValid())
    {
    int aidx = CursorArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, g);
    if (aidx >= 0 && this->m_storage->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      {
      this->m_storage->trigger(StorageEventType(DEL_EVENT, MODEL_INCLUDES_GROUP), *this, g);
      }
    }
  return *this;
}

ModelEntity& ModelEntity::addSubmodel(const ModelEntity& m)
{
  if (this->isValid() && m.isValid() && m.storage() == this->storage() && m.entity() != this->entity())
    {
    CursorArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, m);
    CursorArrangementOps::findOrAddSimpleRelationship(m, SUBSET_OF, *this);
    this->m_storage->trigger(std::make_pair(ADD_EVENT, MODEL_INCLUDES_MODEL), *this, m);
    }
  return *this;
}

ModelEntity& ModelEntity::removeSubmodel(const ModelEntity& m)
{
  if (this->isValid() && m.isValid() && m.storage() == this->storage() && m.entity() != this->entity())
    {
    int aidx = CursorArrangementOps::findSimpleRelationship(*this, SUPERSET_OF, m);
    if (aidx >= 0 && this->m_storage->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      {
      this->m_storage->trigger(StorageEventType(DEL_EVENT, MODEL_INCLUDES_MODEL), *this, m);
      }
    }
  return *this;
}

/// Return an operator of the given \a name with its Storage set to this model's.
OperatorPtr ModelEntity::op(const std::string& name) const
{
  OperatorPtr oper = this->bridge()->op(name);
  if (oper) oper->setStorage(this->m_storage);
  return oper;
}

/// Return a set of the operators available for this model.
Operators ModelEntity::operators() const
{
  Operators ops;
  Operators::const_iterator it;
  BridgeBase::ConstPtr bridge = this->bridge();
  for (
    it = bridge->operators().begin();
    it != bridge->operators().end();
    ++it)
    {
    ops.insert((*it)->clone()->setStorage(this->m_storage));
    }
  return ops;
}

/// Return the names of all the operators which can be applied to this model.
StringList ModelEntity::operatorNames() const
{
  return this->bridge()->operatorNames();
}

BridgeBasePtr ModelEntity::bridge() const
{
  return this->m_storage->bridgeForModel(this->m_entity);
}

  } // namespace model
} // namespace smtk

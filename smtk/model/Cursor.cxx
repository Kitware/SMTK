#include "smtk/model/Cursor.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {

Cursor::Cursor()
{
}

Cursor::Cursor(StoragePtr storage, const smtk::util::UUID& entity)
  : m_storage(storage), m_entity(entity)
{
}

bool Cursor::setStorage(StoragePtr storage)
{
  if (storage == this->m_storage)
    {
    return false;
    }
  this->m_storage = storage;
  return true;
}

StoragePtr Cursor::storage()
{
  return this->m_storage;
}

bool Cursor::setEntity(const smtk::util::UUID& entity)
{
  if (entity == this->m_entity)
    {
    return false;
    }
  this->m_entity = entity;
  return true;
}

smtk::util::UUID Cursor::entity()
{
  return this->m_entity;
}

int Cursor::dimension()
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

int Cursor::dimensionBits()
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

BitFlags Cursor::entityFlags()
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

  } // namespace model
} // namespace smtk

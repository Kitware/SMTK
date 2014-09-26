//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Cursor.h"
#include <iostream>
using namespace smtk::attribute;
//----------------------------------------------------------------------------
Attribute::Attribute(const std::string &myName,
                     smtk::attribute::DefinitionPtr myDefinition,
                     unsigned long myId):
  m_name(myName), m_id(myId), m_definition(myDefinition),
  m_appliesToBoundaryNodes(false), m_appliesToInteriorNodes(false),
  m_isColorSet(false), m_aboutToBeDeleted(false)
{
  this->m_definition->buildAttribute(this);
}

//----------------------------------------------------------------------------
Attribute::~Attribute()
{
  this->m_aboutToBeDeleted = true;
  // Clear all references to the attribute
  std::map<smtk::attribute::RefItem *, std::set<std::size_t> >::iterator it;
  for (it = this->m_references.begin(); it != this->m_references.end(); it++)
    {
    std::set<std::size_t>::iterator sit;
    for (sit = it->second.begin(); sit != it->second.end(); sit++)
      {
      it->first->unset(*sit);
      }
    }
  this->removeAllItems();
 }
//----------------------------------------------------------------------------
void Attribute::removeAllItems()
{
  // we need to detatch all items owned bu this attribute
  std::size_t i, n = this->m_items.size();
  for (i = 0; i < n; i++)
    {
    this->m_items[i]->detachOwningAttribute();
    }
  this->m_items.clear();
}
//----------------------------------------------------------------------------
void Attribute::references(std::vector<smtk::attribute::ItemPtr> &list) const
{
  list.clear();
  std::map<smtk::attribute::RefItem *, std::set<std::size_t> >::const_iterator it;
  for (it = this->m_references.begin(); it != this->m_references.end(); it++)
    {
    if (it->second.size())
      {
      list.push_back(it->first->pointer());
      }
    }
}
//----------------------------------------------------------------------------
const double *Attribute::color() const
{
  if (this->m_isColorSet)
    {
    return this->m_color;
    }
  return this->m_definition->defaultColor();
}
//----------------------------------------------------------------------------
const std::string &Attribute::type() const
{
  return this->m_definition->type();
}
//----------------------------------------------------------------------------
std::vector<std::string> Attribute::types() const
{
  std::vector<std::string> tvec;
  smtk::attribute::DefinitionPtr def = this->m_definition;
  while (def)
    {
    tvec.push_back(def->type());
    def = def->baseDefinition();
    }
  return tvec;
}
//----------------------------------------------------------------------------
bool Attribute::isA(smtk::attribute::DefinitionPtr def) const
{
  return this->m_definition->isA(def);
}
///----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::string &category) const
{
  return this->m_definition->isMemberOf(category);
}
//----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::vector<std::string> &categories) const
{
  return this->m_definition->isMemberOf(categories);
}

namespace {

template<typename T>
bool isInvalid(T itemPtr)
{
  if (!itemPtr)
    return true;
  std::size_t actual = itemPtr->numberOfValues();
  std::size_t minNum = itemPtr->numberOfRequiredValues();
  if (
    actual < minNum ||
    (minNum && actual > minNum))
    return true;

  for (std::size_t i = 0; i < actual; ++i)
    if (!itemPtr->isSet(i))
      return true;
  return false;
}

template<>
bool isInvalid<ValueItemPtr>(ValueItemPtr itemPtr)
{
  if (!itemPtr)
    return true;
  std::size_t actual = itemPtr->numberOfValues();
  std::size_t minNum = itemPtr->numberOfRequiredValues();
  std::size_t maxNum = itemPtr->maxNumberOfValues();
  if (
    actual < minNum ||
    actual > maxNum ||
    (maxNum == 0 && actual > minNum && !itemPtr->isExtensible()))
    return true;

  for (std::size_t i = 0; i < actual; ++i)
    if (!itemPtr->isSet(i))
      return true;
  return false;
}

}

/**\brief Validate the attribute against its definition.
  *
  * This method will only return true when every (required) item in the
  * attribute is set and considered a valid value by its definition.
  * This can be used to ensure that an attribute is in a good state
  * before using it to perform some operation.
  */
bool Attribute::isValid()
{
  std::vector<smtk::attribute::ItemPtr> items;
  this->references(items);
  std::vector<smtk::attribute::ItemPtr>::iterator it;
  for (it = items.begin(); it != items.end(); ++it)
    {
    switch ((*it)->type())
      {
    case Item::ATTRIBUTE_REF:
        {
        smtk::attribute::RefItemPtr ri =
          smtk::dynamic_pointer_cast<smtk::attribute::RefItem>(*it);
        if (isInvalid(ri))
          return false;
        }
      break;
    case Item::MODEL_ENTITY:
        {
        smtk::attribute::ModelEntityItemPtr mei =
          smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItem>(*it);
        if (isInvalid(mei))
          return false;
        }
      break;
    case Item::GROUP:
      break;
    case Item::VOID:
      break;
    case Item::DOUBLE:
    case Item::INT:
    case Item::STRING:
    case Item::FILE:
    case Item::DIRECTORY:
    case Item::COLOR:
        {
        smtk::attribute::ValueItemPtr vi =
          smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(*it);
        if (isInvalid(vi))
          return false;
        }
      break;
    default:
      break;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
Manager *Attribute::manager() const
{
  return this->m_definition->manager();
}
//----------------------------------------------------------------------------
/**\brief Return the model Manager instance whose entities may have attributes.
  *
  * This returns a shared pointer to smtk::model::Manager, which may be
  * null if no manager is referenced by the attribute manager (or if the
  * attribute definition does not reference a valid manager).
  */
smtk::model::ManagerPtr Attribute::modelManager() const
{
  smtk::model::ManagerPtr result;
  smtk::attribute::Manager* attMgr = this->manager();
  if (attMgr)
    {
    result = attMgr->refModelManager();
    }
  return result;
}
//----------------------------------------------------------------------------
smtk::attribute::AttributePtr Attribute::pointer() const
{
  Manager *m = this->manager();
  if (m)
    {
    return m->findAttribute(this->m_name);
    }
  return smtk::attribute::AttributePtr();
}
//----------------------------------------------------------------------------
/**\brief Remove all associations of this attribute with model entities.
  *
  * Note that this removes both new- and old-style associations.
  */
void Attribute::removeAllAssociations()
{
  // new-style model entities
  smtk::model::ManagerPtr modelMgr;
  unsigned long attribId = this->id();
  if (modelMgr)
    {
    smtk::common::UUIDs::const_iterator mit;
    for (
      mit = this->m_modelEntities.begin();
      mit != this->m_modelEntities.end();
      ++mit)
      {
      modelMgr->detachAttribute(attribId, *mit, false);
      }
    }
  this->m_modelEntities.clear();
}
//----------------------------------------------------------------------------
/**\brief Is the model \a entity associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::common::UUID& entity) const
{
  return (this->m_modelEntities.find(entity) != this->m_modelEntities.end());
}
//----------------------------------------------------------------------------
/**\brief Is the model entity of the \a cursor associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::model::Cursor& cursor) const
{
  return this->isEntityAssociated(cursor.entity());
}
//----------------------------------------------------------------------------
/*! \fn template<typename T> T Attribute::associatedModelEntities() const
 *\brief Return a container of associated cursor-subclass instances.
 *
 * This method returns a container (usually a std::vector or std::set) of
 * cursor-subclass instances (e.g., Edge, EdgeUse, Loop) that are
 * associated with this attribute.
 *
 * Note that if you request a container of Cursor entities, you will obtain
 * <b>all</b> of the associated model entities. However, if you request
 * a container of some subclass, only entities of that type will be returned.
 * For example, if an attribute is associated with two faces, an edge,
 * a group, and a shell, calling `associatedModelEntities<Cursors>()` will
 * return 5 Cursor entries while `associatedModelEntities<CellEntities>()`
 * will return 3 entries (2 faces and 1 edge) since the other entities
 * do not construct valid CellEntity instances.
 */
//----------------------------------------------------------------------------
/**\brief Associate a new-style model ID (a UUID) with this attribute.
  *
  * This function returns true when the association is valid and
  * successful. It may return false if the association is prohibited.
  * (This is not currently implemented.)
  */
bool Attribute::associateEntity(const smtk::common::UUID& entity)
{
  if (this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return true; // Entity may be and is now associated
    }
  // TODO: Verify that entity may be associated with this attribute.
  //       If it may not, then we should return false.
  this->m_modelEntities.insert(entity);
  smtk::model::ManagerPtr modelMgr = this->modelManager();
  if (modelMgr)
    modelMgr->attachAttribute(this->id(), entity);
  return true; // Entity may be and is now associated.
}
/**\brief Associate a new-style model ID (a Cursor) with this attribute.
  *
  * This function returns true when the association is valid and
  * successful. It may return false if the association is prohibited.
  * (This is not currently implemented.)
  */
bool Attribute::associateEntity(const smtk::model::Cursor& entity)
{
  return this->associateEntity(entity.entity());
}
//----------------------------------------------------------------------------
/**\brief Disassociate a new-style model ID (a UUID) from this attribute.
  *
  */
void Attribute::disassociateEntity(const smtk::common::UUID& entity, bool reverse)
{
  if (!this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return;
    }

  this->m_modelEntities.erase(entity);
  if(reverse)
    {
    smtk::model::ManagerPtr modelMgr = this->modelManager();
    if (modelMgr)
      {
      modelMgr->detachAttribute(this->id(), entity, false);
      }
    }
}
//----------------------------------------------------------------------------
/**\brief Disassociate a new-style model entity (a Cursor) from this attribute.
  *
  */
void Attribute::disassociateEntity(const smtk::model::Cursor& entity, bool reverse)
{
  this->disassociateEntity(entity.entity(), reverse);
}
//----------------------------------------------------------------------------
smtk::attribute::ConstItemPtr Attribute::find(const std::string &inName) const
{
  int i = this->m_definition->findItemPosition(inName);
  if (i < 0)
    {
    return smtk::attribute::ConstItemPtr();
    }
  return this->m_items[static_cast<std::size_t>(i)];
}

//----------------------------------------------------------------------------
smtk::attribute::ItemPtr Attribute::find(const std::string &inName)
{
  int i = this->m_definition->findItemPosition(inName);
  return (i < 0) ? smtk::attribute::ItemPtr() : this->m_items[static_cast<std::size_t>(i)];
}
//-----------------------------------------------------------------------------
smtk::attribute::IntItemPtr Attribute::findInt(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<IntItem>(this->find(nameStr)); }
smtk::attribute::ConstIntItemPtr Attribute::findInt(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const IntItem>(this->find(nameStr)); }

smtk::attribute::DoubleItemPtr Attribute::findDouble(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<DoubleItem>(this->find(nameStr)); }
smtk::attribute::ConstDoubleItemPtr Attribute::findDouble(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const DoubleItem>(this->find(nameStr)); }

smtk::attribute::StringItemPtr Attribute::findString(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<StringItem>(this->find(nameStr)); }
smtk::attribute::ConstStringItemPtr Attribute::findString(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const StringItem>(this->find(nameStr)); }

smtk::attribute::FileItemPtr Attribute::findFile(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<FileItem>(this->find(nameStr)); }
smtk::attribute::ConstFileItemPtr Attribute::findFile(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const FileItem>(this->find(nameStr)); }

smtk::attribute::DirectoryItemPtr Attribute::findDirectory(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<DirectoryItem>(this->find(nameStr)); }
smtk::attribute::ConstDirectoryItemPtr Attribute::findDirectory(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const DirectoryItem>(this->find(nameStr)); }

smtk::attribute::GroupItemPtr Attribute::findGroup(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<GroupItem>(this->find(nameStr)); }
smtk::attribute::ConstGroupItemPtr Attribute::findGroup(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const GroupItem>(this->find(nameStr)); }

smtk::attribute::RefItemPtr Attribute::findRef(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<RefItem>(this->find(nameStr)); }
smtk::attribute::ConstRefItemPtr Attribute::findRef(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const RefItem>(this->find(nameStr)); }

smtk::attribute::ModelEntityItemPtr Attribute::findModelEntity(const std::string &nameStr)
{ return smtk::dynamic_pointer_cast<ModelEntityItem>(this->find(nameStr)); }
smtk::attribute::ConstModelEntityItemPtr Attribute::findModelEntity(const std::string &nameStr) const
{ return smtk::dynamic_pointer_cast<const ModelEntityItem>(this->find(nameStr)); }

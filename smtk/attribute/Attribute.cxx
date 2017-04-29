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
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/UUIDGenerator.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/algorithm/string.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cassert>
#include <iostream>

using namespace smtk::attribute;
using namespace smtk::common;

Attribute::Attribute(const std::string& myName, smtk::attribute::DefinitionPtr myDefinition,
  const smtk::common::UUID& myId)
  : m_name(myName)
  , m_id(myId)
  , m_definition(myDefinition)
  , m_appliesToBoundaryNodes(false)
  , m_appliesToInteriorNodes(false)
  , m_isColorSet(false)
  , m_aboutToBeDeleted(false)
{
  this->m_definition->buildAttribute(this);
}

Attribute::Attribute(const std::string& myName, smtk::attribute::DefinitionPtr myDefinition)
  : m_name(myName)
  , m_definition(myDefinition)
  , m_appliesToBoundaryNodes(false)
  , m_appliesToInteriorNodes(false)
  , m_isColorSet(false)
  , m_aboutToBeDeleted(false)
{
  smtk::common::UUIDGenerator gen;
  this->m_id = gen.random();
  this->m_definition->buildAttribute(this);
}

Attribute::~Attribute()
{
  this->m_aboutToBeDeleted = true;
  // Clear all references to the attribute
  std::map<smtk::attribute::RefItem*, std::set<std::size_t> >::iterator it;
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

void Attribute::references(std::vector<smtk::attribute::ItemPtr>& list) const
{
  list.clear();
  std::map<smtk::attribute::RefItem*, std::set<std::size_t> >::const_iterator it;
  for (it = this->m_references.begin(); it != this->m_references.end(); it++)
  {
    if (it->second.size())
    {
      list.push_back(it->first->shared_from_this());
    }
  }
}

const double* Attribute::color() const
{
  if (this->m_isColorSet)
  {
    return this->m_color;
  }
  return this->m_definition->defaultColor();
}

const std::string& Attribute::type() const
{
  return this->m_definition->type();
}

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

bool Attribute::isA(smtk::attribute::DefinitionPtr def) const
{
  return this->m_definition->isA(def);
}

bool Attribute::isMemberOf(const std::string& category) const
{
  return this->m_definition->isMemberOf(category);
}

bool Attribute::isMemberOf(const std::vector<std::string>& categories) const
{
  return this->m_definition->isMemberOf(categories);
}

/**\brief Return an item given a string specifying a path to it.
  *
  */
smtk::attribute::ConstItemPtr Attribute::itemAtPath(
  const std::string& path, const std::string& seps) const
{
  smtk::attribute::ConstItemPtr result;
  std::vector<std::string> tree;
  std::vector<std::string>::iterator it;
  boost::split(tree, path, boost::is_any_of(seps));
  if (tree.empty())
  {
    return result;
  }

  it = tree.begin();
  smtk::attribute::ConstItemPtr current = this->find(*it, NO_CHILDREN);
  if (current)
  {
    bool ok = true;
    for (++it; it != tree.end(); ++it)
    {
      ConstValueItemPtr vitm = smtk::dynamic_pointer_cast<const ValueItem>(current);
      ConstGroupItemPtr gitm = smtk::dynamic_pointer_cast<const GroupItem>(current);
      if (vitm && (current = vitm->findChild(*it, NO_CHILDREN)))
        continue; // OK, keep descending
      else if (gitm && (current = gitm->find(*it)))
        continue; // OK, keep descending
      else
      {
        ok = false;
        break;
      }
    }
    if (ok)
    {
      result = current;
    }
  }
  return result;
}

smtk::attribute::ItemPtr Attribute::itemAtPath(const std::string& path, const std::string& seps)
{
  smtk::attribute::ItemPtr result;
  std::vector<std::string> tree;
  std::vector<std::string>::iterator it;
  boost::split(tree, path, boost::is_any_of(seps));
  if (tree.empty())
  {
    return result;
  }

  it = tree.begin();
  smtk::attribute::ItemPtr current = this->find(*it, NO_CHILDREN);
  if (current)
  {
    bool ok = true;
    for (++it; it != tree.end(); ++it)
    {
      ValueItemPtr vitm = smtk::dynamic_pointer_cast<ValueItem>(current);
      GroupItemPtr gitm = smtk::dynamic_pointer_cast<GroupItem>(current);
      if (vitm && (current = vitm->findChild(*it, NO_CHILDREN)))
      {
        continue; // OK, keep descending
      }
      else if (gitm && (current = gitm->find(*it)))
      {
        continue; // OK, keep descending
      }
      else
      {
        ok = false;
        break;
      }
    }
    if (ok)
    {
      result = current;
    }
  }
  return result;
}

/**\brief Validate the attribute against its definition.
  *
  * This method will only return true when every (required) item in the
  * attribute is set and considered a valid value by its definition.
  * This can be used to ensure that an attribute is in a good state
  * before using it to perform some operation.
  */
bool Attribute::isValid() const
{
  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    if (!(*it)->isValid())
    {
      return false;
    }
  }
  // also check associations
  if (this->m_associations && !this->m_associations->isValid())
  {
    return false;
  }
  return true;
}

SystemPtr Attribute::system() const
{
  return this->m_definition->system();
}

/**\brief Return the model Manager instance whose entities may have attributes.
  *
  * This returns a shared pointer to smtk::model::Manager, which may be
  * null if no manager is referenced by the attribute system (or if the
  * attribute definition does not reference a valid system).
  */
smtk::model::ManagerPtr Attribute::modelManager() const
{
  smtk::model::ManagerPtr result;
  smtk::attribute::SystemPtr attSys = this->system();
  if (attSys)
  {
    result = attSys->refModelManager();
  }
  return result;
}

/**\brief Remove all associations of this attribute with model entities.
  *
  * Note that this actually resets the associations.
  * If there are any default associations, they will be present
  * but typically there are none.
  */
void Attribute::removeAllAssociations()
{
  smtk::model::ManagerPtr modelMgr = this->modelManager();
  if (modelMgr && this->m_associations)
  {
    smtk::model::EntityRefArray::const_iterator it;
    for (it = this->m_associations->begin(); it != this->m_associations->end(); ++it)
    {
      modelMgr->disassociateAttribute(this->system(), this->m_id, it->entity(), false);
    }
  }

  if (this->m_associations)
  {
    this->m_associations->reset();
  }
}

/**\brief Is the model \a entity associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::common::UUID& entity) const
{
  return this->m_associations ? this->m_associations->has(entity) : false;
}

/**\brief Is the model entity of the \a entityref associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::model::EntityRef& entityref) const
{
  return this->m_associations ? this->m_associations->has(entityref) : false;
}

/**\brief Return the associated model entities as a set of UUIDs.
  *
  */
smtk::common::UUIDs Attribute::associatedModelEntityIds() const
{
  smtk::common::UUIDs result;
  if (!this->m_associations)
  {
    return result;
  }

  smtk::model::EntityRefArray::const_iterator it;
  for (it = this->m_associations->begin(); it != this->m_associations->end(); ++it)
  {
    result.insert(it->entity());
  }
  return result;
}

/*! \fn template<typename T> T Attribute::associatedModelEntities() const
 *\brief Return a container of associated entityref-subclass instances.
 *
 * This method returns a container (usually a std::vector or std::set) of
 * entityref-subclass instances (e.g., Edge, EdgeUse, Loop) that are
 * associated with this attribute.
 *
 * Note that if you request a container of EntityRef entities, you will obtain
 * <b>all</b> of the associated model entities. However, if you request
 * a container of some subclass, only entities of that type will be returned.
 * For example, if an attribute is associated with two faces, an edge,
 * a group, and a shell, calling `associatedModelEntities<EntityRefs>()` will
 * return 5 EntityRef entries while `associatedModelEntities<CellEntities>()`
 * will return 3 entries (2 faces and 1 edge) since the other entities
 * do not construct valid CellEntity instances.
 */

/**\brief Associate a new-style model ID (a UUID) with this attribute.
  *
  * This function returns true when the association is valid and
  * successful. It may return false if the association is prohibited.
  * (This is not currently implemented.)
  */
bool Attribute::associateEntity(const smtk::common::UUID& entity)
{
  smtk::model::ManagerPtr modelMgr = this->modelManager();
  return this->associateEntity(smtk::model::EntityRef(modelMgr, entity));
}

/**\brief Associate a new-style model ID (a EntityRef) with this attribute.
  *
  * This function returns true when the association is valid and
  * successful. It may return false if the association is prohibited.
  * (This is not currently implemented.)
  */
bool Attribute::associateEntity(const smtk::model::EntityRef& entityRef)
{
  bool res = this->isEntityAssociated(entityRef);
  if (res)
    return res;

  res = (this->m_associations) ? this->m_associations->appendValue(entityRef) : false;
  if (!res)
    return res;

  smtk::model::ManagerPtr modelMgr = this->modelManager();
  if (!modelMgr)
  {
    modelMgr = entityRef.manager();
  }
  if (modelMgr)
  {
    res = modelMgr->associateAttribute(this->system(), this->id(), entityRef.entity());
  }
  return res;
}

/**\brief Disassociate a new-style model ID (a UUID) from this attribute.
  *
  * If \a reverse is true (the default), then the model manager is notified
  * of the item's disassociation immediately after its removal from this
  * attribute, allowing the model and attribute to stay in sync.
  */
void Attribute::disassociateEntity(const smtk::common::UUID& entity, bool reverse)
{
  if (!this->m_associations)
  {
    return;
  }

  std::ptrdiff_t idx = this->m_associations->find(entity);
  if (idx >= 0)
  {
    this->m_associations->removeValue(idx);
    if (reverse)
    {
      smtk::model::ManagerPtr modelMgr = this->modelManager();
      if (modelMgr)
      {
        modelMgr->disassociateAttribute(this->system(), this->id(), entity, false);
      }
    }
  }
}

/**\brief Disassociate a new-style model entity (a EntityRef) from this attribute.
  *
  */
void Attribute::disassociateEntity(const smtk::model::EntityRef& entity, bool reverse)
{
  if (!this->m_associations)
  {
    return;
  }

  std::ptrdiff_t idx = this->m_associations->find(entity);
  if (idx >= 0)
  {
    this->m_associations->removeValue(idx);
    if (reverse)
    {
      smtk::model::EntityRef mutableEntity(entity);
      mutableEntity.disassociateAttribute(this->system(), this->id(), false);
    }
  }
}

/**\brief Return the item with the given \a inName, searching in the given \a style.
  *
  * The search style dictates whether children of conditional items are included
  * and, if so, whether all of their children are searched or just the active children.
  * The default is to search active children.
  */
smtk::attribute::ItemPtr Attribute::find(const std::string& inName, SearchStyle style)
{
  int i = this->m_definition->findItemPosition(inName);
  if (i < 0 && style != NO_CHILDREN)
  { // try to find child items that match the name and type.
    std::size_t numItems = this->numberOfItems();
    for (i = 0; i < static_cast<int>(numItems); ++i)
    {
      ValueItem::Ptr vitem = dynamic_pointer_cast<ValueItem>(this->item(i));
      Item::Ptr match;
      if (vitem && (match = vitem->findChild(inName, style)))
      {
        return match;
      }
    }
    i = -1; // Nothing found.
  }
  assert(i < 0 || this->m_items.size() > static_cast<std::size_t>(i));
  return (i < 0) ? smtk::attribute::ItemPtr() : this->m_items[static_cast<std::size_t>(i)];
}

smtk::attribute::ConstItemPtr Attribute::find(const std::string& inName, SearchStyle style) const
{
  int i = this->m_definition->findItemPosition(inName);
  if (i < 0 && style != NO_CHILDREN)
  { // try to find child items that match the name and type.
    std::size_t numItems = this->numberOfItems();
    for (i = 0; i < static_cast<int>(numItems); ++i)
    {
      ConstValueItemPtr vitem = dynamic_pointer_cast<const ValueItem>(this->item(i));
      ConstItemPtr match;
      if (vitem && (match = vitem->findChild(inName, style)))
      {
        return match;
      }
    }
    i = -1; // Nothing found.
  }
  assert(i < 0 || this->m_items.size() > static_cast<std::size_t>(i));
  return (i < 0) ? smtk::attribute::ConstItemPtr()
                 : smtk::const_pointer_cast<const Item>(this->m_items[static_cast<std::size_t>(i)]);
}

smtk::attribute::IntItemPtr Attribute::findInt(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<IntItem>(this->find(nameStr));
}
smtk::attribute::ConstIntItemPtr Attribute::findInt(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const IntItem>(this->find(nameStr));
}

smtk::attribute::DoubleItemPtr Attribute::findDouble(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<DoubleItem>(this->find(nameStr));
}
smtk::attribute::ConstDoubleItemPtr Attribute::findDouble(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const DoubleItem>(this->find(nameStr));
}

smtk::attribute::StringItemPtr Attribute::findString(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<StringItem>(this->find(nameStr));
}
smtk::attribute::ConstStringItemPtr Attribute::findString(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const StringItem>(this->find(nameStr));
}

smtk::attribute::FileItemPtr Attribute::findFile(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<FileItem>(this->find(nameStr));
}
smtk::attribute::ConstFileItemPtr Attribute::findFile(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const FileItem>(this->find(nameStr));
}

smtk::attribute::DirectoryItemPtr Attribute::findDirectory(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<DirectoryItem>(this->find(nameStr));
}
smtk::attribute::ConstDirectoryItemPtr Attribute::findDirectory(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const DirectoryItem>(this->find(nameStr));
}

smtk::attribute::GroupItemPtr Attribute::findGroup(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<GroupItem>(this->find(nameStr));
}
smtk::attribute::ConstGroupItemPtr Attribute::findGroup(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const GroupItem>(this->find(nameStr));
}

smtk::attribute::RefItemPtr Attribute::findRef(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<RefItem>(this->find(nameStr));
}
smtk::attribute::ConstRefItemPtr Attribute::findRef(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const RefItem>(this->find(nameStr));
}

smtk::attribute::ModelEntityItemPtr Attribute::findModelEntity(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<ModelEntityItem>(this->find(nameStr));
}
smtk::attribute::ConstModelEntityItemPtr Attribute::findModelEntity(
  const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const ModelEntityItem>(this->find(nameStr));
}

smtk::attribute::VoidItemPtr Attribute::findVoid(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<VoidItem>(this->find(nameStr));
}
smtk::attribute::ConstVoidItemPtr Attribute::findVoid(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const VoidItem>(this->find(nameStr));
}

smtk::attribute::MeshSelectionItemPtr Attribute::findMeshSelection(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<MeshSelectionItem>(this->find(nameStr));
}
smtk::attribute::ConstMeshSelectionItemPtr Attribute::findMeshSelection(
  const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const MeshSelectionItem>(this->find(nameStr));
}

smtk::attribute::MeshItemPtr Attribute::findMesh(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<MeshItem>(this->find(nameStr));
}
smtk::attribute::ConstMeshItemPtr Attribute::findMesh(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const MeshItem>(this->find(nameStr));
}

smtk::attribute::DateTimeItemPtr Attribute::findDateTime(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<DateTimeItem>(this->find(nameStr));
}
smtk::attribute::ConstDateTimeItemPtr Attribute::findDateTime(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const DateTimeItem>(this->find(nameStr));
}

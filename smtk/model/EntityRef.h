//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EntityRef_h
#define __smtk_model_EntityRef_h
/*! \file */

#include "smtk/CoreExports.h" // For EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For WeakManagerPtr
#include "smtk/SystemConfig.h" // For type macros.

#include "smtk/common/UUID.h"

#include "smtk/model/Arrangement.h" // for ArrangementKind and Arrangements types
#include "smtk/model/AttributeAssignments.h" // for BitFlags type
#include "smtk/model/EntityTypeBits.h" // for BitFlags type
#include "smtk/model/Events.h" // for ManagerEventRelationType type
#include "smtk/model/FloatData.h" // for Float, FloatData, ...
#include "smtk/model/IntegerData.h" // for Integer, IntegerData, ...
#include "smtk/model/PropertyType.h" // for PropertyType
#include "smtk/model/StringData.h" // for String, StringData, ...

#include <iostream>
#include <set>
#include <vector>

/// A macro to implement mandatory EntityRef-subclass constructors.
#define SMTK_ENTITYREF_CLASS(thisclass,superclass,typecheck) \
  SMTK_DERIVED_TYPE(thisclass,superclass); \
  thisclass () {} \
  thisclass (const EntityRef& other) \
    : superclass(other) {} \
  thisclass (ManagerPtr inManager, const smtk::common::UUID& entityId) \
    : superclass(inManager, entityId) {} \
  bool isValid() const { return this->EntityRef::isValid(); } \
  virtual bool isValid(Entity** entRec) const \
    { \
      Entity* er; \
      if ( \
        /* NB: EntityRef::isValid() may return true even when er == NULL */ \
        this->EntityRef::isValid(&er) && er && \
        smtk::model:: typecheck (er->entityFlags())) \
        { \
        if (entRec) *entRec = er; \
        return true; \
        } \
      return false; \
    } \
  /* Required for shiboken: */ \
  bool operator == (const EntityRef& other) const \
    { return this->superclass::operator == (other); } \
  bool operator != (const EntityRef& other) const \
    { return this->superclass::operator != (other); }

namespace smtk {
  namespace model {

class EntityRef;
class Model;
class Tessellation;
// Use full names including namespace to make Shiboken less unhappy:
typedef std::set<smtk::model::EntityRef> EntityRefs;
typedef std::vector<smtk::model::EntityRef> EntityRefArray;
typedef std::vector<Group> Groups;

/**\brief A lightweight entityref pointing to a model entity's manager.
  *
  * This class exposes methods from multiple members of the model
  * manager that are all associated with a given entity.
  * See Manager, Manager, Entity, EntityTypeBits, and other
  * headers for documentation on the methods in this class.
  *
  * It is a convenience class only and new functionality added to
  * Manager and other smtk::model classes should not rely on it.
  */
class SMTKCORE_EXPORT EntityRef
{
public:
  SMTK_BASE_TYPE(EntityRef);
  EntityRef();
  EntityRef(ManagerPtr manager, const smtk::common::UUID& entityId);

  bool setManager(ManagerPtr manager);
  ManagerPtr manager();
  const ManagerPtr manager() const;

  bool setEntity(const smtk::common::UUID& entityId);
  const smtk::common::UUID& entity() const;

  int dimension() const;
  int dimensionBits() const;
  void setDimensionBits(BitFlags dim);
  BitFlags entityFlags() const;
  std::string flagSummary(int form = 0) const;

  int maxParametricDimension() const;

  int embeddingDimension() const;

  std::string name() const;
  void setName(const std::string& n);
  std::string assignDefaultName(bool overwrite = false);

  bool hasVisibility() const;
  bool visible() const;
  void setVisible(bool vis);

  FloatList color() const;
  bool hasColor() const;
  void setColor(const FloatList& rgba);
  void setColor(double r, double g, double b, double a = 1.);

  bool isValid() const;
  virtual bool isValid(Entity** entityRecord) const;
  virtual bool checkForArrangements(ArrangementKind k, Entity*& entry, Arrangements*& arr) const;

  bool isCellEntity()  const { return smtk::model::isCellEntity(this->entityFlags()); }
  bool isUseEntity()   const { return smtk::model::isUseEntity(this->entityFlags()); }
  bool isShellEntity() const { return smtk::model::isShellEntity(this->entityFlags()); }
  bool isGroup()       const { return smtk::model::isGroup(this->entityFlags()); }
  bool isModel()       const { return smtk::model::isModel(this->entityFlags()); }
  bool isInstance()    const { return smtk::model::isInstance(this->entityFlags()); }
  bool isSessionRef()  const { return smtk::model::isSessionRef(this->entityFlags()); }
  bool isConcept()     const { return smtk::model::isConcept(this->entityFlags()); }

  bool isVertex()    const { return smtk::model::isVertex(this->entityFlags()); }
  bool isEdge()      const { return smtk::model::isEdge(this->entityFlags()); }
  bool isFace()      const { return smtk::model::isFace(this->entityFlags()); }
  bool isVolume()    const { return smtk::model::isVolume(this->entityFlags()); }
  bool isChain()     const { return smtk::model::isChain(this->entityFlags()); }
  bool isLoop()      const { return smtk::model::isLoop(this->entityFlags()); }
  bool isShell()     const { return smtk::model::isShell(this->entityFlags()); }
  bool isVertexUse() const { return smtk::model::isVertexUse(this->entityFlags()); }
  bool isEdgeUse()   const { return smtk::model::isEdgeUse(this->entityFlags()); }
  bool isFaceUse()   const { return smtk::model::isFaceUse(this->entityFlags()); }
  bool isVolumeUse() const { return smtk::model::isVolumeUse(this->entityFlags()); }

  /**\brief Reinterpret a entityref as a subclass.
    *
    * Note that you should call isValid() on the resulting
    * entityref subclass to perform sanity checks; this method
    * will happily return an invalid object.
    */
  template<typename T>
  T as() const
    {
    return T(*this);
    }

  template<typename S, typename T>
  static void EntityRefsFromUUIDs(
    S& result, ManagerPtr, const T& uids);

  template<typename S, typename T>
  static void EntityRefsToUUIDs(
    S& uids, const T& entRefs);

  EntityRefs bordantEntities(int ofDimension = -2) const;
  EntityRefs boundaryEntities(int ofDimension = -2) const;

  EntityRefs lowerDimensionalBoundaries(int lowerDimension);
  EntityRefs higherDimensionalBordants(int higherDimension);
  EntityRefs adjacentEntities(int ofDimension);

  template<typename T> T relationsAs() const;
  EntityRefs relations() const;
  EntityRef& addRawRelation(const EntityRef& ent);
  EntityRef& findOrAddRawRelation(const EntityRef& ent);
  EntityRef& elideRawRelation(const EntityRef& ent);

  const Tessellation* hasTessellation() const;
  const Tessellation* hasAnalysisMesh() const;
  const Tessellation* gotMesh() const; //prefers the analaysis over the display
  int setTessellation(const Tessellation* tess, int analysisMesh = 0);
  bool removeTessellation(bool removeGen = false);
  void findEntitiesWithTessellation(
    std::map<smtk::model::EntityRef, smtk::model::EntityRef>& entityrefMap,
    std::set<smtk::model::EntityRef>& touched) const;

  bool hasAttributes() const;
  bool hasAttribute(const smtk::common::UUID &attribId) const;
  bool associateAttribute(smtk::attribute::System* sys, const smtk::common::UUID &attribId);
  bool disassociateAttribute(smtk::attribute::System* sys,
                             const smtk::common::UUID &attribId, bool reverse = true);
  bool disassociateAllAttributes(smtk::attribute::System* sys, bool reverse = true);

  AttributeSet attributes() const;

#ifndef SHIBOKEN_SKIP
  // For T = {IntegerData, FloatData, StringData}:
  template<typename T> T* properties();
  template<typename T> T* hasProperties();
  template<typename T> const T* hasProperties() const;
  template<typename T> bool removeProperty(const std::string& name);
#endif // SHIBOKEN_SKIP

  void setFloatProperty(const std::string& propName, smtk::model::Float propValue);
  void setFloatProperty(const std::string& propName, const smtk::model::FloatList& propValue);
  smtk::model::FloatList const& floatProperty(const std::string& propName) const;
  smtk::model::FloatList& floatProperty(const std::string& propName);
  bool hasFloatProperty(const std::string& propName) const;
  bool removeFloatProperty(const std::string& propName);
  bool hasFloatProperties() const;
  std::set<std::string> floatPropertyNames() const;
  FloatData& floatProperties();
  FloatData const& floatProperties() const;

  void setStringProperty(const std::string& propName, const smtk::model::String& propValue);
  void setStringProperty(const std::string& propName, const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(const std::string& propName) const;
  smtk::model::StringList& stringProperty(const std::string& propName);
  bool hasStringProperty(const std::string& propName) const;
  bool removeStringProperty(const std::string& propName);
  bool hasStringProperties() const;
  std::set<std::string> stringPropertyNames() const;
  StringData& stringProperties();
  StringData const& stringProperties() const;

  void setIntegerProperty(const std::string& propName, smtk::model::Integer propValue);
  void setIntegerProperty(const std::string& propName, const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(const std::string& propName);
  bool hasIntegerProperty(const std::string& propName) const;
  bool removeIntegerProperty(const std::string& propName);
  bool hasIntegerProperties() const;
  std::set<std::string> integerPropertyNames() const;
  IntegerData& integerProperties();
  IntegerData const& integerProperties() const;

  int numberOfArrangementsOfKind(ArrangementKind k) const;
  Arrangement* findArrangement(ArrangementKind k, int index);
  const Arrangement* findArrangement(ArrangementKind k, int index) const;
  bool clearArrangements();

  EntityRef relationFromArrangement(ArrangementKind k, int arrangementIndex, int offset) const;

  bool removeArrangement(ArrangementKind k, int index = -1);

  // Manage embedded_in/includes relationships
  EntityRef& embedEntity(const EntityRef& thingToEmbed);
  template<typename T> EntityRef& embedEntities(const T& container);
  bool isEmbedded(EntityRef& ent) const;
  EntityRef embeddedIn() const;
  bool unembedEntity(const EntityRef& thingToUnembed);
  template<typename T> EntityRef& unembedEntities(const T& container);

  template<typename T> T instances() const;

  Model owningModel() const;
  SessionRef owningSession() const;
  Groups containingGroups() const;

  bool operator == (const EntityRef& other) const;
  bool operator != (const EntityRef& other) const;
  bool operator < (const EntityRef& other) const;

  std::size_t hash() const;

protected:
  friend class Group;
  friend class Model;
  friend class SessionRef;

  WeakManagerPtr m_manager;
  smtk::common::UUID m_entity;

  // Manage subset_of/superset_of relationships
  EntityRef& addMemberEntity(const EntityRef& memberToAdd);
  template<typename T> EntityRef& addMemberEntities(T begin, T end);
  bool isMember(EntityRef& entity) const;
  EntityRef memberOf() const;
  EntityRef& removeMemberEntity(const EntityRef& memberToRemove);
  EntityRef& removeMemberEntity(int indexOfMemberToRemove);
  template<typename T> EntityRef& removeMemberEntities(T begin, T end);

  ManagerEventRelationType subsetRelationType(const EntityRef& member) const;
  ManagerEventRelationType embeddingRelationType(const EntityRef& embedded) const;
};

SMTKCORE_EXPORT std::ostream& operator << (std::ostream& os, const EntityRef& c);

SMTKCORE_EXPORT std::size_t entityrefHash(const EntityRef& c);

template<typename T>
T EntityRef::relationsAs() const
{
  T result;
  ManagerPtr mgr = this->m_manager.lock();
  smtk::model::Entity* entRec;
  if (!this->isValid(&entRec))
    return result;

  smtk::common::UUIDArray::const_iterator it;
  for (it = entRec->relations().begin(); it != entRec->relations().end(); ++it)
    {
    typename T::value_type entry(mgr, *it);
    if (entry.isValid())
      {
      result.insert(result.end(), entry);
      }
    }
  return result;
}

template<typename S, typename T>
void EntityRef::EntityRefsFromUUIDs(
  S& result, ManagerPtr mgr, const T& uids)
{
  for (typename T::const_iterator it = uids.begin(); it != uids.end(); ++it)
    {
    typename S::value_type entry(mgr, *it);
    if (entry.isValid())
      {
      result.insert(result.end(), entry);
      }
    }
}

template<typename S, typename T>
void EntityRef::EntityRefsToUUIDs(
  S& uids, const T& entRefs)
{
  for (typename T::const_iterator it = entRefs.begin(); it != entRefs.end(); ++it)
    {
    if (it->entity())
      {
      uids.insert(uids.end(), it->entity());
      }
    }
}

template<typename T> EntityRef& EntityRef::embedEntities(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->embedEntity(*it);
    }
  return *this;
}

template<typename T> EntityRef& EntityRef::unembedEntities(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->unembedEntity(*it);
    }
  return *this;
}

template<typename T> EntityRef& EntityRef::addMemberEntities(T begin, T end)
{
  for (T it = begin; it != end; ++it)
    this->addMemberEntity(*it);
  return *this;
}

template<typename T> EntityRef& EntityRef::removeMemberEntities(T begin, T end)
{
  for (T it = begin; it != end; ++it)
    this->removeMemberEntity(*it);
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EntityRef_h

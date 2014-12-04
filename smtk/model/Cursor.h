//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Cursor_h
#define __smtk_model_Cursor_h
/*! \file */

#include "smtk/SMTKCoreExports.h" // For EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For ManagerPtr
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

/// A macro to implement mandatory Cursor-subclass constructors.
#define SMTK_CURSOR_CLASS(thisclass,superclass,typecheck) \
  SMTK_DERIVED_TYPE(thisclass,superclass); \
  thisclass () {} \
  thisclass (const Cursor& other) \
    : superclass(other) {} \
  thisclass (ManagerPtr inManager, const smtk::common::UUID& entityId) \
    : superclass(inManager, entityId) {} \
  bool isValid() const { return this->Cursor::isValid(); } \
  virtual bool isValid(Entity** entRec) const \
    { \
      Entity* er; \
      if ( \
        /* NB: Cursor::isValid() may return true even when er == NULL */ \
        this->Cursor::isValid(&er) && er && \
        smtk::model:: typecheck (er->entityFlags())) \
        { \
        if (entRec) *entRec = er; \
        return true; \
        } \
      return false; \
    } \
  /* Required for shiboken: */ \
  bool operator == (const Cursor& other) const \
    { return this->superclass::operator == (other); }

namespace smtk {
  namespace model {

class Cursor;
class ModelEntity;
class Tessellation;
// Use full names including namespace to make Shiboken less unhappy:
typedef std::set<smtk::model::Cursor> Cursors;
typedef std::vector<smtk::model::Cursor> CursorArray;

/**\brief A lightweight cursor pointing to a model entity's manager.
  *
  * This class exposes methods from multiple members of the model
  * manager that are all associated with a given entity.
  * See BRepModel, Manager, Entity, EntityTypeBits, and other
  * headers for documentation on the methods in this class.
  *
  * It is a convenience class only and new functionality added to
  * Manager and other smtk::model classes should not rely on it.
  */
class SMTKCORE_EXPORT Cursor
{
public:
  SMTK_BASE_TYPE(Cursor);
  Cursor();
  Cursor(ManagerPtr manager, const smtk::common::UUID& entityId);

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
  std::string assignDefaultName();

  FloatList color() const;
  bool hasColor() const;
  void setColor(const FloatList& rgba);
  void setColor(double r, double g, double b, double a = 1.);

  bool isValid() const;
  virtual bool isValid(Entity** entityRecord) const;
  virtual bool checkForArrangements(ArrangementKind k, Entity*& entry, Arrangements*& arr) const;

  bool isCellEntity()     const { return smtk::model::isCellEntity(this->entityFlags()); }
  bool isUseEntity()      const { return smtk::model::isUseEntity(this->entityFlags()); }
  bool isShellEntity()    const { return smtk::model::isShellEntity(this->entityFlags()); }
  bool isGroupEntity()    const { return smtk::model::isGroupEntity(this->entityFlags()); }
  bool isModelEntity()    const { return smtk::model::isModelEntity(this->entityFlags()); }
  bool isInstanceEntity() const { return smtk::model::isInstanceEntity(this->entityFlags()); }

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

  /**\brief Reinterpret a cursor as a subclass.
    *
    * Note that you should call isValid() on the resulting
    * cursor subclass to perform sanity checks; this method
    * will happily return an invalid object.
    */
  template<typename T>
  T as() const
    {
    return T(*this);
    }

  template<typename S, typename T>
  static void CursorsFromUUIDs(
    S& result, ManagerPtr, const T& uids);

  Cursors bordantEntities(int ofDimension = -2) const;
  Cursors boundaryEntities(int ofDimension = -2) const;

  Cursors lowerDimensionalBoundaries(int lowerDimension);
  Cursors higherDimensionalBordants(int higherDimension);
  Cursors adjacentEntities(int ofDimension);

  template<typename T> T relationsAs() const;
  Cursors relations() const;
  Cursor& addRawRelation(const Cursor& ent);
  Cursor& findOrAddRawRelation(const Cursor& ent);

  const Tessellation* hasTessellation() const;

  bool hasAttributes() const;
  bool hasAttribute(const smtk::common::UUID &attribId) const;
  bool associateAttribute(const smtk::common::UUID &attribId);
  bool disassociateAttribute(const smtk::common::UUID &attribId, bool reverse = true);
  AttributeAssignments& attributes();
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

  Cursor relationFromArrangement(ArrangementKind k, int arrangementIndex, int offset) const;

  Cursor& embedEntity(const Cursor& thingToEmbed);
  template<typename T> Cursor& embedEntities(const T& container);
  bool isEmbedded(Cursor& entity) const;
  Cursor embeddedIn() const;

  Cursor& unembedEntity(const Cursor& thingToUnembed);
  template<typename T> Cursor& unembedEntities(const T& container);

  template<typename T> T instances() const;

  ModelEntity owningModel() const;

  bool operator == (const Cursor& other) const;
  bool operator < (const Cursor& other) const;

  std::size_t hash() const;

protected:
  ManagerPtr m_manager;
  smtk::common::UUID m_entity;

  // When embedding/unembedding, this method determines the relationship type
  // based on the entities involved.
  ManagerEventRelationType embeddingRelationType(const Cursor& embedded) const;
};

SMTKCORE_EXPORT std::ostream& operator << (std::ostream& os, const Cursor& c);

SMTKCORE_EXPORT std::size_t cursorHash(const Cursor& c);

template<typename T>
T Cursor::relationsAs() const
{
  T result;
  smtk::model::Entity* entRec;
  if (!this->isValid(&entRec))
    return result;

  smtk::common::UUIDArray::const_iterator it;
  for (it = entRec->relations().begin(); it != entRec->relations().end(); ++it)
    {
    typename T::value_type entry(this->m_manager, *it);
    if (entry.isValid())
      {
      result.insert(result.end(), entry);
      }
    }
  return result;
}

template<typename S, typename T>
void Cursor::CursorsFromUUIDs(
  S& result, ManagerPtr mgr, const T& uids)
{
  typename S::size_type expected = 1;
  for (typename T::const_iterator it = uids.begin(); it != uids.end(); ++it, ++expected)
    {
    typename S::value_type entry(mgr, *it);
    if (entry.isValid())
      {
      result.insert(result.end(), entry);
      }
    }
}

template<typename T> Cursor& Cursor::embedEntities(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->embedEntity(*it);
    }
  return *this;
}

template<typename T> Cursor& Cursor::unembedEntities(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->unembedEntity(*it);
    }
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Cursor_h

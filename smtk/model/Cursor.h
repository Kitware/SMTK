#ifndef __smtk_model_Cursor_h
#define __smtk_model_Cursor_h

#include "smtk/SMTKCoreExports.h" // For EXPORT macro.
#include "smtk/util/SystemConfig.h" // For type macros.
#include "smtk/PublicPointerDefs.h" // For StoragePtr
#include "smtk/model/AttributeAssignments.h" // for BitFlags type
#include "smtk/model/EntityTypeBits.h" // for BitFlags type
#include "smtk/model/FloatData.h" // for Float, FloatData, ...
#include "smtk/model/StringData.h" // for String, StringData, ...
#include "smtk/model/IntegerData.h" // for Integer, IntegerData, ...
#include "smtk/model/Storage.h"

#include "smtk/util/UUID.h"

#include <iostream>
#include <set>
#include <vector>

/// A macro to implement mandatory Cursor-subclass constructors.
#define SMTK_CURSOR_CLASS(thisclass,superclass,typecheck) \
  SMTK_DERIVED_TYPE(thisclass,superclass); \
  thisclass () {} \
  thisclass (const Cursor& other) \
    : superclass(other) {} \
  thisclass (StoragePtr inStorage, const smtk::util::UUID& entityId) \
    : superclass(inStorage, entityId) {} \
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
    }

namespace smtk {
  namespace model {

class Cursor;
typedef std::set<Cursor> Cursors;
typedef std::vector<Cursor> CursorArray;

/**\brief A lightweight cursor pointing to a model entity's storage.
  *
  * This class exposes methods from multiple pieces of storage
  * that are all associated with a given entity.
  * See BRepModel, Storage, Entity, EntityTypeBits, and other
  * headers for documentation on the methods in this class.
  *
  * It is a convenience class only and new functionality added to
  * Storage and other smtk::model classes should not rely on it.
  */
class SMTKCORE_EXPORT Cursor
{
public:
  SMTK_BASE_TYPE(Cursor);
  Cursor();
  Cursor(StoragePtr storage, const smtk::util::UUID& entityId);

  bool setStorage(StoragePtr storage);
  StoragePtr storage();
  const StoragePtr storage() const;

  bool setEntity(const smtk::util::UUID& entityId);
  const smtk::util::UUID& entity() const;

  int dimension() const;
  int dimensionBits() const;
  BitFlags entityFlags() const;
  std::string flagSummary(int form = 0) const;
  std::string name() const;

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
  T as()
    {
    return T(*this);
    }

  template<typename S, typename T>
  static void CursorsFromUUIDs(
    S& result, StoragePtr, const T& uids);

  Cursors bordantEntities(int ofDimension = -2) const;
  Cursors boundaryEntities(int ofDimension = -2) const;

  Cursors lowerDimensionalBoundaries(int lowerDimension);
  Cursors higherDimensionalBordants(int higherDimension);
  Cursors adjacentEntities(int ofDimension);

  bool hasAttributes() const;
  bool hasAttribute(int attribId) const;
  bool attachAttribute(int attribId);
  bool detachAttribute(int attribId, bool reverse = true);
  AttributeAssignments& attributes();
  AttributeAssignments::AttributeSet attributes() const;

  void setFloatProperty(const std::string& propName, smtk::model::Float propValue);
  void setFloatProperty(const std::string& propName, const smtk::model::FloatList& propValue);
  smtk::model::FloatList const& floatProperty(const std::string& propName) const;
  smtk::model::FloatList& floatProperty(const std::string& propName);
  bool hasFloatProperty(const std::string& propName) const;
  bool removeFloatProperty(const std::string& propName);
  bool hasFloatProperties() const;
  FloatData& floatProperties();
  FloatData const& floatProperties() const;

  void setStringProperty(const std::string& propName, const smtk::model::String& propValue);
  void setStringProperty(const std::string& propName, const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(const std::string& propName) const;
  smtk::model::StringList& stringProperty(const std::string& propName);
  bool hasStringProperty(const std::string& propName) const;
  bool removeStringProperty(const std::string& propName);
  bool hasStringProperties() const;
  StringData& stringProperties();
  StringData const& stringProperties() const;

  void setIntegerProperty(const std::string& propName, smtk::model::Integer propValue);
  void setIntegerProperty(const std::string& propName, const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(const std::string& propName);
  bool hasIntegerProperty(const std::string& propName) const;
  bool removeIntegerProperty(const std::string& propName);
  bool hasIntegerProperties() const;
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

  template<typename T> T instances() const;

  bool operator == (const Cursor& other) const;
  bool operator < (const Cursor& other) const;

protected:
  StoragePtr m_storage;
  smtk::util::UUID m_entity;
};

SMTKCORE_EXPORT std::ostream& operator << (std::ostream& os, const Cursor& c);

template<typename S, typename T>
void Cursor::CursorsFromUUIDs(
  S& result, StoragePtr storage, const T& uids)
{
  typename S::size_type expected = 1;
  for (typename T::const_iterator it = uids.begin(); it != uids.end(); ++it, ++expected)
    {
    typename S::value_type entry(storage, *it);
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

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Cursor_h

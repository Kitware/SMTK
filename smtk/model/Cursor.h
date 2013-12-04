#ifndef __smtk_model_Cursor_h
#define __smtk_model_Cursor_h

#include "smtk/SMTKCoreExports.h" // For EXPORT macro.
#include "smtk/util/SystemConfig.h" // For type macros.
#include "smtk/PublicPointerDefs.h" // For StoragePtr
#include "smtk/model/EntityTypeBits.h" // for BitFlags type
#include "smtk/model/FloatData.h" // for Float, FloatData, ...
#include "smtk/model/StringData.h" // for String, StringData, ...
#include "smtk/model/IntegerData.h" // for Integer, IntegerData, ...
#include "smtk/model/Storage.h"

#include "smtk/util/UUID.h"

#include <set>
#include <vector>

/// A macro to implement mandatory Cursor-subclass constructors.
#define SMTK_CURSOR_CLASS(thisclass,superclass,typecheck) \
  SMTK_DERIVED_TYPE(thisclass,superclass); \
  thisclass () {} \
  thisclass (const Cursor& other) \
    : superclass(other) {} \
  thisclass (StoragePtr storage, const smtk::util::UUID& entity) \
    : superclass(storage, entity) {} \
  bool isValid() const { return this->Cursor::isValid(); } \
  virtual bool isValid(Entity** entRec) const \
    { \
      Entity* er; \
      if ( \
        this->Cursor::isValid(&er) && \
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
  Cursor(StoragePtr storage, const smtk::util::UUID& entity);

  bool setStorage(StoragePtr storage);
  StoragePtr storage();

  bool setEntity(const smtk::util::UUID& entity);
  smtk::util::UUID entity();

  int dimension() const;
  int dimensionBits() const;
  BitFlags entityFlags() const;

  /**\brief Return whether the cursor is pointing to valid storage that contains the UUID of the entity.
    *
    * Subclasses should not override this method. It is a convenience
    * which makes the shiboken wrapper more functional.
    */
  bool isValid() const
    {
    return this->isValid(NULL);
    }

  /**\brief Return whether the cursor is pointing to valid storage that contains the UUID of the entity.
    *
    * Subclasses override this and additionally return whether the entity is of
    * a type that matches the Cursor subclass. For example, it is possible to
    * create a Vertex cursor from a UUID referring to an EdgeUse. While
    * Cursor::isValid() will return true, Vertex::isValid() will return false.
    *
    * The optional \a entityRecord will be set when a non-NULL value is passed
    * and the entity is valid.
    */
  virtual bool isValid(Entity** entityRecord) const
    {
    bool status = this->m_storage && !this->m_entity.isNull();
    if (status && entityRecord)
      {
      *entityRecord = this->m_storage->findEntity(this->m_entity);
      }
    return status;
    }

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

  template<typename T>
  T as()
    {
    return T(*this);
    }

  static void CursorsFromUUIDs(
    Cursors& result, StoragePtr, const smtk::util::UUIDs& uids);

  Cursors bordantEntities(int ofDimension = -2);
  Cursors boundaryEntities(int ofDimension = -2);

  Cursors lowerDimensionalBoundaries(int lowerDimension);
  Cursors higherDimensionalBordants(int higherDimension);
  Cursors adjacentEntities(int ofDimension);

  void setFloatProperty(const std::string& propName, smtk::model::Float propValue);
  void setFloatProperty(const std::string& propName, const smtk::model::FloatList& propValue);
  smtk::model::FloatList const& floatProperty(const std::string& propName) const;
  smtk::model::FloatList& floatProperty(const std::string& propName);
  bool hasFloatProperty(const std::string& propName) const;
  bool removeFloatProperty(const std::string& propName);
  FloatData& floatProperties();
  FloatData const& floatProperties() const;

  void setStringProperty(const std::string& propName, const smtk::model::String& propValue);
  void setStringProperty(const std::string& propName, const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(const std::string& propName) const;
  smtk::model::StringList& stringProperty(const std::string& propName);
  bool hasStringProperty(const std::string& propName) const;
  bool removeStringProperty(const std::string& propName);
  StringData& stringProperties();
  StringData const& stringProperties() const;

  void setIntegerProperty(const std::string& propName, smtk::model::Integer propValue);
  void setIntegerProperty(const std::string& propName, const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(const std::string& propName);
  bool hasIntegerProperty(const std::string& propName) const;
  bool removeIntegerProperty(const std::string& propName);
  IntegerData& integerProperties();
  IntegerData const& integerProperties() const;

  bool operator < (const Cursor& other) const;

protected:
  StoragePtr m_storage;
  smtk::util::UUID m_entity;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Cursor_h

#ifndef __smtk_model_BridgeBase_h
#define __smtk_model_BridgeBase_h

#include "smtk/util/UUID.h"
#include "smtk/util/SharedFromThis.h"
#include "smtk/util/SystemConfig.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/SharedPtr.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/Operator.h"

namespace smtk {
  namespace model {

class BridgeBase;
class Cursor;
class Operator;
typedef std::map<smtk::util::UUID,smtk::shared_ptr<BridgeBase> > UUIDsToBridges;

/**\brief Bit flags describing types of information bridged to Storage.
  *
  * Bridge classes should provide individual translation for
  * each piece of information, but are allowed to transcribe
  * additional information when it is efficient or necessary
  * to do so.
  * For example, it does not make sense for an Entity record's
  * relations to be populated but not the bit-flag describing
  * its type. Thus, requesting BRIDGE_ENTITY_RELATIONS usually
  * also results in BRIDGE_ENTITY_TYPE being transcribed.
  */
enum BridgedInformation
{
  // Basic types of information in smtk::model::Storage
  BRIDGE_ENTITY_TYPE            = 0x00000001, //!< Transcribe the Entity type.
  BRIDGE_ENTITY_RELATIONS       = 0x00000002, //!< Transcribe the Entity relationship vector.
  BRIDGE_ARRANGEMENTS           = 0x00000004, //!< Arrangement information for the relationships.
  BRIDGE_TESSELLATION           = 0x00000008, //!< Points and triangles used to render an entity.
  BRIDGE_FLOAT_PROPERTIES       = 0x00000010, //!< Floating-point properties.
  BRIDGE_STRING_PROPERTIES      = 0x00000020, //!< String properties.
  BRIDGE_INTEGER_PROPERTIES     = 0x00000040, //!< Integer properties.
  BRIDGE_ATTRIBUTE_ASSOCIATIONS = 0x00000080, //!< Attribute associations.

  // Common combinations
  BRIDGE_ENTITY_RECORD          = 0x00000003, //!< Transcribe both entity type and relations.
  BRIDGE_ENTITY_ARRANGED        = 0x00000007, //!< Transcribe the entity record and all arrangement information.
  BRIDGE_PROPERTIES             = 0x00000070, //!< Transcribe all properties.
  BRIDGE_EVERYTHING             = 0x000000ff  //!< Transcribe all information about the entity.
};

/// Bit-vector combinations of BridgedInformation values for requesting information to transcribe.
typedef unsigned long BridgedInfoBits;

/**\brief A base class for bridging modelers into SMTK.
  *
  * SMTK can act as a bridge between other (foreign) solid modelers
  * and client applications.
  * Either the bridge or the foreign modeler must provide techniques
  * for attaching UUIDs to foreign model entities and for obtaining
  * notification when foreign model entities are modified or
  * destroyed. In extreme cases, SMTK Storage must be reset after
  * each modeling operation to guarantee a consistent model.
  *
  * Bridges may provide SMTK with Operators that can be used to
  * modify models in storage.
  *
  * Register an instance of a BridgeBase subclass to a
  * model with Storage::bridgeModel(). Then, when an
  * entity cannot be resolved from a UUID created by
  * the bridge, the \a transcribe method will be invoked
  * to request that the bridge add an entry.
  *
  * This class is not intended for external use.
  * Public methods are intended for invocation by the
  * Storage instance which owns the bridge.
  * Protected methods are either called internally or
  * by subclasses in order to track UUIDs for which there
  * is only partial information in Storage.
  *
  * \sa smtk::model::BridgedInformation smtk::model::Operator
  */
class SMTKCORE_EXPORT BridgeBase : smtkEnableSharedPtr(BridgeBase)
{
public:
  smtkTypeMacro(BridgeBase);

  int transcribe(const Cursor& entity, BridgedInfoBits flags, bool onlyDangling = true);

  virtual BridgedInfoBits allSupportedInformation() const;

  StringList operatorNames() const;
  const Operators& operators() const;
  OperatorPtr op(const std::string& opName) const;
  virtual void addOperator(OperatorPtr op);

protected:
  void declareDanglingEntity(const Cursor& ent, BridgedInfoBits present = 0);

  virtual BridgedInfoBits transcribeInternal(const Cursor& entity, BridgedInfoBits flags);

  typedef std::map<smtk::model::Cursor,int> DanglingEntities;
  DanglingEntities m_dangling;
  Operators m_operators;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_BridgeBase_h

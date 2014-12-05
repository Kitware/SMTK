//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Bridge_h
#define __smtk_model_Bridge_h
/*! \file */

#include "smtk/SystemConfig.h"
#include "smtk/SharedPtr.h"
#include "smtk/SharedFromThis.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include "smtk/attribute/System.h"

#include "smtk/model/BridgeRegistrar.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/Operator.h"

namespace smtk {
  namespace model {

/// Bit-vector combinations of BridgedInformation values for requesting information to transcribe.
typedef unsigned long BridgedInfoBits;

class Bridge;
class Cursor;
class Operator;
typedef std::map<smtk::common::UUID,smtk::shared_ptr<Bridge> > UUIDsToBridges;
typedef std::map<smtk::model::Cursor,BridgedInfoBits> DanglingEntities;

/**\brief Bit flags describing types of information bridged to Manager.
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
  // Basic types of information in smtk::model::Manager
  BRIDGE_ENTITY_TYPE            = 0x00000001, //!< Transcribe the Entity type.
  BRIDGE_ENTITY_RELATIONS       = 0x00000002, //!< Transcribe the Entity relationship vector.
  BRIDGE_ARRANGEMENTS           = 0x00000004, //!< Arrangement information for the relationships.
  BRIDGE_TESSELLATION           = 0x00000008, //!< Points and triangles used to render an entity.
  BRIDGE_FLOAT_PROPERTIES       = 0x00000010, //!< Floating-point properties.
  BRIDGE_STRING_PROPERTIES      = 0x00000020, //!< String properties.
  BRIDGE_INTEGER_PROPERTIES     = 0x00000040, //!< Integer properties.
  BRIDGE_ATTRIBUTE_ASSOCIATIONS = 0x00000080, //!< Attribute associations.

  // Common combinations
  BRIDGE_NOTHING                = 0x00000000, //!< Transcribe nothing.
  BRIDGE_ENTITY_RECORD          = 0x00000003, //!< Transcribe both entity type and relations.
  BRIDGE_ENTITY_ARRANGED        = 0x00000007, //!< Transcribe the entity record and all arrangement information.
  BRIDGE_PROPERTIES             = 0x00000070, //!< Transcribe all properties.
  BRIDGE_EVERYTHING             = 0x000000ff  //!< Transcribe all information about the entity.
};

#ifndef SHIBOKEN_SKIP

/**\brief Boilerplate for classes that accept operator registration.
 *
 * This is invoked by smtkDeclareModelingKernel(), so you should not
 * normally need to use it directly.
 *
 * Note that you must invoke this macro in a public section of your class declaration!
 *
 * If you invoke this macro, you must also use the
 * smtkImplementsOperatorRegistration macro in your bridge's implementation.
 */
#define smtkDeclareOperatorRegistration() \
protected: \
  static void cleanupOperators(); \
  static smtk::model::OperatorConstructors* s_operators; \
public: \
  virtual bool registerOperator( \
    const std::string& opName, const char* opDescrXML, \
    smtk::model::OperatorConstructor opCtor); \
  static bool registerStaticOperator( \
    const std::string& opName, const char* opDescrXML, \
    smtk::model::OperatorConstructor opCtor); \
  virtual std::string findOperatorXML(const std::string& opName) const; \
  virtual smtk::model::OperatorConstructor findOperatorConstructor( \
    const std::string& opName) const; \
  virtual bool inheritsOperators() const;

/**\brief Implement methods declared by smtkDeclareOperatorRegistration().
  *
  * Usually this macro is invoked by smtkImplementsModelingKernel().
  */
#define smtkImplementsOperatorRegistration(Cls, Inherits) \
  /**\brief Declare the map of operator constructors */ \
  smtk::model::OperatorConstructors* Cls ::s_operators = NULL; \
  /**\brief Virtual method to allow operators to register themselves with us */ \
  bool Cls ::registerOperator( \
    const std::string& opName, const char* opDescrXML, \
    smtk::model::OperatorConstructor opCtor) \
  { \
    bool result = Cls ::registerStaticOperator(opName, opDescrXML, opCtor); \
    if (opDescrXML) \
      this->importOperatorXML(opDescrXML); \
    return result; \
  } \
  /**\brief Allow operators to register themselves with us */ \
  bool Cls ::registerStaticOperator( \
    const std::string& opName, const char* opDescrXML, \
    smtk::model::OperatorConstructor opCtor) \
  { \
  if (!Cls ::s_operators) \
    { \
    Cls ::s_operators = new smtk::model::OperatorConstructors; \
    atexit(Cls ::cleanupOperators); \
    } \
  if (!opName.empty() && opCtor) \
    { \
    smtk::model::StaticOperatorInfo entry(opDescrXML ? opDescrXML : "",opCtor); \
    (* Cls ::s_operators)[opName] = entry; \
    return true; \
    } \
  else if (!opName.empty()) \
    { /* unregister the operator of the given name. */ \
    Cls ::s_operators->erase(opName); \
    /* FIXME: We should ensure that no operator instances of this type are in */ \
    /*        existence before allowing "unregistration" to proceed. */ \
    } \
  return false; \
  } \
  /**\brief Find an operator constructor in this subclass' static list. */ \
  smtk::model::OperatorConstructor Cls ::findOperatorConstructor( \
    const std::string& opName) const \
  { \
    smtk::model::OperatorConstructor result; \
    result = this->findOperatorConstructorInternal(opName, Cls ::s_operators); \
    if (!result && this->inheritsOperators() && \
      Cls::Superclass::staticClassName() != Cls::staticClassName()) \
      result = this->Superclass::findOperatorConstructor(opName); \
    return result; \
  } \
  /**\brief Find an XML description of an operator in this subclass' static list. */ \
  std::string Cls ::findOperatorXML(const std::string& opName) const \
  { \
    std::string result; \
    result = this->findOperatorXMLInternal(opName, Cls ::s_operators); \
    if (result.empty() && this->inheritsOperators() && \
      Cls::Superclass::staticClassName() != Cls::staticClassName()) \
      result = this->Superclass::findOperatorXML(opName); \
    return result; \
  } \
  /**\brief Called to delete registered operator map at exit. */ \
  void Cls ::cleanupOperators() \
  { \
    delete Cls ::s_operators; \
    Cls ::s_operators = NULL; \
  } \
  /**\brief Return whether the class inherits operators. */ \
  bool Cls ::inheritsOperators() const { return Inherits; }

/**\brief Boilerplate for classes that bridge to a solid modeling kernel.
 *
 * Invoke this macro inside every class definition inheriting smtk::model::Bridge.
 * Both smtk/model/DefaultBridge.{h,cxx} and smtk/cgm/Bridge.{h,cxx} are examples.
 * Note that you must invoke this macro in a public section of your class declaration!
 *
 * You must also use the smtkImplementsModelingKernel macro in your bridge's implementation.
 */
#define smtkDeclareModelingKernel() \
  static std::string bridgeName; \
  static std::string staticClassName(); \
  virtual std::string name() const { return bridgeName; } \
  virtual std::string className() const; \
  smtkDeclareOperatorRegistration();

/**\brief Declare that a class implements a bridge to a solid modeling kernel.
  *
  * Invoke this macro inside every class definition inheriting smtk::model::Bridge.
  * Both smtk/model/DefaultBridge.{h,cxx} and smtk/cgm/Bridge.{h,cxx} are examples.
  *
  * Note that you must invoke this macro in the global namespace!
  *
  * You must also use the smtkDeclareModelingKernel macro in your bridge's header.
  *
  * This macro takes 3 arguments:
  *
  * \a Comp      - A "short" name for the bridge. This is used as part of several function
  *                names, so it must be a valid variable name and should *not* be in quotes.
  * \a Tags      - A pointer to a NULL-terminated string containing a JSON description of
  *                the bridge's capabilities, including file types that the bridge supports.
  *                The format of the JSON structure is documented in the SMTK User's Guide.
  * \a Setup     - A function to provide configuration before bridge construction.
  *                See the documentation for BridgeStaticSetup and BridgeHasNoStaticSetup.
  * \a Cls       - The name of the bridge class. The class must have a static method named
  *                "create" that constructs and instance and returns a shared pointer to it.
  * \a Inherits  - Either "true" or "false", depending on whether the bridge should inherit
  *                operators from its superclass. This is used to keep forwarding bridges
  *                like the Remus remote bridge from inheriting local operators.
  */
#define smtkImplementsModelingKernel(Comp, Tags, Setup, Cls, Inherits) \
  /* Adapt create() to return a base-class pointer */ \
  static smtk::model::BridgePtr baseCreate() { \
    return Cls ::create(); \
  } \
  /* Implement autoinit methods */ \
  void smtk_##Comp##_bridge_AutoInit_Construct() { \
    smtk::model::BridgeRegistrar::registerBridge( \
      #Comp, /* Can't rely on bridgeName to be initialized yet */ \
      Tags, \
      Setup, \
      baseCreate); \
  } \
  void smtk_##Comp##_bridge_AutoInit_Destruct() { \
    smtk::model::BridgeRegistrar::registerBridge( \
      Cls ::bridgeName, \
      std::string(), \
      NULL, \
      NULL); \
  } \
  /**\brief Declare the component name */ \
  std::string Cls ::bridgeName(#Comp); \
  /**\brief Return the name of this class. */\
  std::string Cls ::staticClassName() { return #Cls ; } \
  /**\brief Declare the class name */ \
  std::string Cls ::className() const { return Cls ::staticClassName(); } \
  smtkImplementsOperatorRegistration(Cls, Inherits); \
  smtkComponentInitMacro(smtk_ ##Comp## _bridge);

#else // SHIBOKEN_SKIP

#define smtkDeclareOperatorRegistration()

#define smtkImplementsOperatorRegistration()

#define smtkImplementsModelingKernel(Comp, Tags, Setup, Cls) \
  std::string Cls ::bridgeName(#Comp); \
  std::string Cls ::staticClassName() { return #Cls ; } \
  std::string Cls ::className() const; \
  std::string Cls ::findOperatorXML(const std::string& opName) const;

#define smtkDeclareModelingKernel() \
  static std::string bridgeName; \
  static std::string staticClassName(); \
  virtual std::string name(); \
  virtual std::string className() const;

#endif // SHIBOKEN_SKIP

/**\brief A base class for bridging modelers into SMTK.
  *
  * SMTK can act as a bridge between other (foreign) solid modelers
  * and client applications.
  * Either the bridge or the foreign modeler must provide techniques
  * for attaching UUIDs to foreign model entities and for obtaining
  * notification when foreign model entities are modified or
  * destroyed. In extreme cases, the SMTK model manager must be reset after
  * each modeling operation to guarantee a consistent model.
  *
  * Bridges may provide SMTK with Operators that can be used to
  * modify models in storage.
  * Operators have two parts: (1) a concrete subclass of smtk::model::Operator
  * that implements the modeling operation using the "foreign" modeling
  * kernel, and (2) a pair of smtk::attribute::Definition instances that
  * define the structure of operator parameters and results.
  * The latter Definition instances are kept inside an attribute system
  * owned by the Bridge; you can access it with smtk::Bridge::operatorSystem().
  *
  * Instances of Bridge subclasses should be registered with a
  * model using Manager::bridgeModel(). Then, when an
  * entity cannot be resolved from a UUID created by
  * the bridge, the \a transcribe method will be invoked
  * to request that the bridge add an entry.
  *
  * This class is not intended for external use.
  * Public methods are intended for invocation by the
  * Manager instance which owns the bridge.
  * Protected methods are either called internally or
  * by subclasses in order to track UUIDs for which there
  * is only partial information in Manager.
  *
  * \sa smtk::model::BridgedInformation smtk::model::Operator
  * \sa smtkDeclareModelingKernel smtkImplementsModelingKernel
  */
class SMTKCORE_EXPORT Bridge : smtkEnableSharedPtr(Bridge)
{
public:
  smtkTypeMacro(Bridge);
  smtkDeclareOperatorRegistration();

  // Required to be circular for Superclass::findOperator{Constructor,XML}:
  smtkSuperclassMacro(smtk::model::Bridge);

  static std::string staticClassName() { return "smtk::model::Bridge"; }
  virtual std::string name() const;
  virtual std::string className() const { return Bridge::staticClassName(); }
  smtk::common::UUID sessionId() const;

  int transcribe(const Cursor& entity, BridgedInfoBits flags, bool onlyDangling = true);

  virtual BridgedInfoBits allSupportedInformation() const;

  StringList operatorNames() const;
  virtual OperatorPtr op(const std::string& opName) const;

  const DanglingEntities& danglingEntities() const;
  void declareDanglingEntity(const Cursor& ent, BridgedInfoBits present = 0);

  smtk::attribute::System* operatorSystem();
  const smtk::attribute::System* operatorSystem() const;

  virtual int setup(const std::string& optName, const StringList& optVal);

  ManagerPtr manager() const;

protected:
  friend class io::ExportJSON;
  friend class io::ImportJSON;
  friend class BRepModel;

  Bridge();
  virtual ~Bridge();

  virtual BridgedInfoBits transcribeInternal(const Cursor& entity, BridgedInfoBits flags);

  void setSessionId(const smtk::common::UUID& sessId);
  void setManager(Manager* mgr);

#ifndef SHIBOKEN_SKIP
  void initializeOperatorSystem(const OperatorConstructors* opList);
  void importOperatorXML(const std::string& opXML);
  virtual OperatorConstructor findOperatorConstructorInternal(const std::string&, const OperatorConstructors* opList) const;
  virtual std::string findOperatorXMLInternal(const std::string&, const OperatorConstructors* opList) const;
#endif // SHIBOKEN_SKIP

  virtual BridgeIOPtr createIODelegate(const std::string& format);

  DanglingEntities m_dangling;
  smtk::common::UUID m_sessionId;
  smtk::attribute::System* m_operatorSys;
  Manager* m_manager;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Bridge_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Session_h
#define __smtk_model_Session_h
/*! \file */

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include "smtk/attribute/System.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/SessionRegistrar.h"

namespace smtk
{
namespace io
{
class Logger;
}
namespace model
{

/// Bit-vector combinations of SessionInformation values for requesting information to transcribe.
typedef unsigned long SessionInfoBits;

class ArrangementHelper;
class EntityRef;
class Group;
class CellEntity;
class UseEntity;
class Instance;
class ShellEntity;
class Model;
class Operator;
class Session;
class SessionRef;
typedef std::map<smtk::common::UUID, smtk::shared_ptr<Session> > UUIDsToSessions;
typedef std::map<smtk::model::EntityRef, SessionInfoBits> DanglingEntities;

/**\brief Bit flags describing types of information bridged to Manager.
  *
  * Session classes should provide individual translation for
  * each piece of information, but are allowed to transcribe
  * additional information when it is efficient or necessary
  * to do so.
  * For example, it does not make sense for an Entity record's
  * relations to be populated but not the bit-flag describing
  * its type. Thus, requesting SESSION_ENTITY_RELATIONS usually
  * also results in SESSION_ENTITY_TYPE being transcribed.
  */
enum SessionInformation
{
  // Basic types of information in smtk::model::Manager
  SESSION_ENTITY_TYPE = 0x00000001,            //!< Transcribe the Entity type.
  SESSION_ENTITY_RELATIONS = 0x00000002,       //!< Transcribe the Entity relationship vector.
  SESSION_ARRANGEMENTS = 0x00000004,           //!< Arrangement information for the relationships.
  SESSION_TESSELLATION = 0x00000008,           //!< Points and triangles used to render an entity.
  SESSION_FLOAT_PROPERTIES = 0x00000010,       //!< Floating-point properties.
  SESSION_STRING_PROPERTIES = 0x00000020,      //!< String properties.
  SESSION_INTEGER_PROPERTIES = 0x00000040,     //!< Integer properties.
  SESSION_ATTRIBUTE_ASSOCIATIONS = 0x00000080, //!< Attribute associations.

  // Extended options specific to Manager::erase():
  SESSION_USER_DEFINED_PROPERTIES =
    0x00000100, /**< Remove user-defined as well as machine-generated properties.
                                                 *
                                                 *  This bit is not used during transcription; it is only
                                                 *  significant when erasing entities. Usually, it is not
                                                 *  specified so that properties such as user-assigned
                                                 *  names, colors, and visibility are preserved.
                                                 */
  // Common combinations
  SESSION_NOTHING = 0x00000000,       //!< Transcribe nothing.
  SESSION_ENTITY_RECORD = 0x00000003, //!< Transcribe both entity type and relations.
  SESSION_ENTITY_ARRANGED =
    0x00000007, //!< Transcribe the entity record and all arrangement information.
  SESSION_PROPERTIES = 0x00000070, //!< Transcribe all properties.
  SESSION_EVERYTHING = 0x000000ff, //!< Transcribe all information about the entity.
  SESSION_EXHAUSTIVE =
    0x000001ff //!< Erase **all** information about the entity, including user-specified.
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
 * smtkImplementsOperatorRegistration macro in your session's implementation.
 */
#define smtkDeclareOperatorRegistration()                                                          \
protected:                                                                                         \
  static void cleanupOperators();                                                                  \
                                                                                                   \
public:                                                                                            \
  static smtk::model::OperatorConstructors* s_operators;                                           \
  virtual bool registerOperator(                                                                   \
    const std::string& opName, const char* opDescrXML, smtk::model::OperatorConstructor opCtor);   \
  static bool registerStaticOperator(                                                              \
    const std::string& opName, const char* opDescrXML, smtk::model::OperatorConstructor opCtor);   \
  virtual std::string findOperatorXML(const std::string& opName) const;                            \
  virtual smtk::model::OperatorConstructor findOperatorConstructor(const std::string& opName)      \
    const;                                                                                         \
  virtual smtk::model::OperatorConstructors* operatorConstructors();                               \
  virtual bool inheritsOperators() const

/**\brief Implement methods declared by smtkDeclareOperatorRegistration().
  *
  * Usually this macro is invoked by smtkImplementsModelingKernel().
  */
#define smtkImplementsOperatorRegistration(Cls, Inherits)                                             \
  /**\brief Declare the map of operator constructors */                                               \
  smtk::model::OperatorConstructors* Cls::s_operators = NULL;                                         \
  /**\brief Override the virtual method returning all operator constructors for this session type. */ \
  smtk::model::OperatorConstructors* Cls::operatorConstructors() { return Cls::s_operators; }         \
  /**\brief Virtual method to allow operators to register themselves with us */                       \
  bool Cls::registerOperator(                                                                         \
    const std::string& opName, const char* opDescrXML, smtk::model::OperatorConstructor opCtor)       \
  {                                                                                                   \
    /* std::cerr << "Register " << opName << " w/ " << #Cls << "\n"; */                               \
    bool result = Cls::registerStaticOperator(opName, opDescrXML, opCtor);                            \
    if (opDescrXML)                                                                                   \
      this->importOperatorXML(opDescrXML);                                                            \
    return result;                                                                                    \
  }                                                                                                   \
  /**\brief Allow operators to register themselves with us */                                         \
  bool Cls::registerStaticOperator(                                                                   \
    const std::string& opName, const char* opDescrXML, smtk::model::OperatorConstructor opCtor)       \
  {                                                                                                   \
    if (!Cls::s_operators)                                                                            \
    {                                                                                                 \
      Cls::s_operators = new smtk::model::OperatorConstructors;                                       \
      atexit(Cls::cleanupOperators);                                                                  \
    }                                                                                                 \
    if (!opName.empty() && opCtor)                                                                    \
    {                                                                                                 \
      smtk::model::StaticOperatorInfo entry(opDescrXML ? opDescrXML : "", opCtor);                    \
      (*Cls::s_operators)[opName] = entry;                                                            \
      return true;                                                                                    \
    }                                                                                                 \
    else if (!opName.empty())                                                                         \
    { /* unregister the operator of the given name. */                                                \
      Cls::s_operators->erase(opName);                                                                \
      /* FIXME: We should ensure that no operator instances of this type are in */                    \
      /*        existence before allowing "unregistration" to proceed. */                             \
    }                                                                                                 \
    return false;                                                                                     \
  }                                                                                                   \
  /**\brief Find an operator constructor in this subclass' static list. */                            \
  smtk::model::OperatorConstructor Cls::findOperatorConstructor(const std::string& opName) const      \
  {                                                                                                   \
    smtk::model::OperatorConstructor result;                                                          \
    result = this->findOperatorConstructorInternal(opName, Cls::s_operators);                         \
    if (!result && this->inheritsOperators() &&                                                       \
      Cls::Superclass::staticClassName() != Cls::staticClassName())                                   \
      result = this->Superclass::findOperatorConstructor(opName);                                     \
    return result;                                                                                    \
  }                                                                                                   \
  /**\brief Find an XML description of an operator in this subclass' static list. */                  \
  std::string Cls::findOperatorXML(const std::string& opName) const                                   \
  {                                                                                                   \
    std::string result;                                                                               \
    result = this->findOperatorXMLInternal(opName, Cls::s_operators);                                 \
    if (result.empty() && this->inheritsOperators() &&                                                \
      Cls::Superclass::staticClassName() != Cls::staticClassName())                                   \
      result = this->Superclass::findOperatorXML(opName);                                             \
    return result;                                                                                    \
  }                                                                                                   \
  /**\brief Called to delete registered operator map at exit. */                                      \
  void Cls::cleanupOperators()                                                                        \
  {                                                                                                   \
    delete Cls::s_operators;                                                                          \
    Cls::s_operators = NULL;                                                                          \
  }                                                                                                   \
  /**\brief Return whether the class inherits operators. */                                           \
  bool Cls::inheritsOperators() const { return Inherits; }

/**\brief Boilerplate for classes that session to a solid modeling kernel.
 *
 * Invoke this macro inside every class definition inheriting smtk::model::Session.
 * Both smtk/model/DefaultSession.{h,cxx} and smtk/cgm/Session.{h,cxx} are examples.
 * Note that you must invoke this macro in a public section of your class declaration!
 *
 * You must also use the smtkImplementsModelingKernel macro in your session's implementation.
 */
#define smtkDeclareModelingKernel()                                                                \
  static std::string sessionName;                                                                  \
  static std::string staticClassName();                                                            \
  virtual std::string name() const { return sessionName; }                                         \
  virtual std::string className() const;                                                           \
  smtkDeclareOperatorRegistration()

/**\brief Declare that a class implements a session to a solid modeling kernel.
  *
  * Invoke this macro inside every class definition inheriting smtk::model::Session.
  * Both smtk/model/DefaultSession.{h,cxx} and smtk/cgm/Session.{h,cxx} are examples.
  *
  * Note that you must invoke this macro in the global namespace!
  *
  * You must also use the smtkDeclareModelingKernel macro in your session's header.
  *
  * This macro takes 6 arguments:
  *
  * \a ExportSym - The symbol used to export the AutoInit functions.
  * \a Comp      - A "short" name for the session. This is used as part of several function
  *                names, so it must be a valid variable name and should *not* be in quotes.
  * \a Tags      - A pointer to a NULL-terminated string containing a JSON description of
  *                the session's capabilities, including file types that the session supports.
  *                The format of the JSON structure is documented in the SMTK User's Guide.
  * \a Setup     - A function to provide configuration before session construction.
  *                See the documentation for SessionStaticSetup and SessionHasNoStaticSetup.
  * \a Cls       - The name of the session class. The class must have a static method named
  *                "create" that constructs and instance and returns a shared pointer to it.
  * \a Inherits  - Either "true" or "false", depending on whether the session should inherit
  *                operators from its superclass. This is used to keep forwarding sessions
  *                like the Remus remote session from inheriting local operators.
  * \a OpCons    - A pointer to an OperatorConstructors instance (i.e., a pointer to a map
  *                from strings (operator names) to StaticOperatorInfo objects, each of
  *                which holds both the operator's XML description and a function pointer
  *                for constructing a subclass of smtk::model::Operator to perform the
  *                operation).
  */
#define smtkImplementsModelingKernel(ExportSym, Comp, Tags, Setup, Cls, Inherits)                  \
  /* Adapt create() to return a base-class pointer */                                              \
  static smtk::model::SessionPtr baseCreate() { return Cls::create(); }                            \
  /* Implement autoinit methods */                                                                 \
  void ExportSym smtk_##Comp##_session_AutoInit_Construct()                                        \
  {                                                                                                \
    smtk::model::SessionRegistrar::registerSession(                                                \
      #Comp, /* Can't rely on sessionName to be initialized yet */                                 \
      Tags, Setup, baseCreate, Cls::s_operators);                                                  \
  }                                                                                                \
  void ExportSym smtk_##Comp##_session_AutoInit_Destruct()                                         \
  {                                                                                                \
    smtk::model::SessionRegistrar::registerSession(Cls::sessionName, std::string(),                \
      SMTK_FUNCTION_INIT, SMTK_FUNCTION_INIT, SMTK_FUNCTION_INIT);                                 \
  }                                                                                                \
  /**\brief Declare the component name */                                                          \
  std::string Cls::sessionName(#Comp);                                                             \
  /**\brief Return the name of this class. */                                                      \
  std::string Cls::staticClassName() { return #Cls; }                                              \
  /**\brief Declare the class name */                                                              \
  std::string Cls::className() const { return Cls::staticClassName(); }                            \
  smtkImplementsOperatorRegistration(Cls, Inherits);                                               \
  smtkComponentInitMacro(smtk_##Comp##_session);

#else // SHIBOKEN_SKIP

#define smtkDeclareOperatorRegistration()

#define smtkImplementsOperatorRegistration()

#define smtkImplementsModelingKernel(Comp, Tags, Setup, Cls)                                       \
  std::string Cls::sessionName(#Comp);                                                             \
  std::string Cls::staticClassName() { return #Cls; }                                              \
  std::string Cls::className() const;                                                              \
  std::string Cls::findOperatorXML(const std::string& opName) const;

#define smtkDeclareModelingKernel()                                                                \
  static std::string sessionName;                                                                  \
  static std::string staticClassName();                                                            \
  virtual std::string name();                                                                      \
  virtual std::string className() const;

#endif // SHIBOKEN_SKIP

/**\brief A base class for bridging modelers into SMTK.
  *
  * SMTK can act as a session between other (foreign) solid modelers
  * and client applications.
  * Either the session or the foreign modeler must provide techniques
  * for attaching UUIDs to foreign model entities and for obtaining
  * notification when foreign model entities are modified or
  * destroyed. In extreme cases, the SMTK model manager must be reset after
  * each modeling operation to guarantee a consistent model.
  *
  * Sessions may provide SMTK with Operators that can be used to
  * modify models in storage.
  * Operators have two parts: (1) a concrete subclass of smtk::model::Operator
  * that implements the modeling operation using the "foreign" modeling
  * kernel, and (2) a pair of smtk::attribute::Definition instances that
  * define the structure of operator parameters and results.
  * The latter Definition instances are kept inside an attribute system
  * owned by the Session; you can access it with smtk::Session::operatorSystem().
  *
  * Instances of Session subclasses should be registered with a
  * model using Manager::sessionModel(). Then, when an
  * entity cannot be resolved from a UUID created by
  * the session, the \a transcribe method will be invoked
  * to request that the session add an entry.
  *
  * This class is not intended for external use.
  * Public methods are intended for invocation by the
  * Manager instance which owns the session.
  * Protected methods are either called internally or
  * by subclasses in order to track UUIDs for which there
  * is only partial information in Manager.
  *
  * \sa smtk::model::SessionInformation smtk::model::Operator
  * \sa smtkDeclareModelingKernel smtkImplementsModelingKernel
  */
class SMTKCORE_EXPORT Session : smtkEnableSharedPtr(Session)
{
public:
  smtkTypeMacro(Session);
  smtkDeclareOperatorRegistration();

  // Required to be circular for Superclass::findOperator{Constructor,XML}:
  smtkSuperclassMacro(smtk::model::Session);

  static std::string staticClassName() { return "smtk::model::Session"; }
  virtual std::string name() const;
  virtual std::string className() const { return Session::staticClassName(); }
  smtk::common::UUID sessionId() const;

  int transcribe(
    const EntityRef& entity, SessionInfoBits flags, bool onlyDangling = true, int depth = -1);

  virtual SessionInfoBits allSupportedInformation() const;

  StringList operatorNames(bool includeAdvanced = true) const;
  std::map<std::string, std::string> operatorLabelsMap(bool includeAdvanced = true) const;
  virtual OperatorPtr op(const std::string& opName) const;

  const DanglingEntities& danglingEntities() const;
  void declareDanglingEntity(const EntityRef& ent, SessionInfoBits present = 0);

  smtk::attribute::SystemPtr operatorSystem();
  smtk::attribute::ConstSystemPtr operatorSystem() const;

  virtual int setup(const std::string& optName, const StringList& optVal);

  ManagerPtr manager() const;
  smtk::mesh::ManagerPtr meshManager() const;
  smtk::io::Logger& log();

  virtual ~Session();

  virtual bool removeGeneratedProperties(const EntityRef& ent, SessionInfoBits propFlags);
  virtual bool splitProperties(const EntityRef& from, const EntityRefs& to) const;
  virtual bool mergeProperties(const EntityRefs& from, EntityRef& to) const;

protected:
  friend class io::SaveJSON;
  friend class io::LoadJSON;
  friend class Manager;

  Session();

  virtual SessionInfoBits transcribeInternal(
    const EntityRef& entity, SessionInfoBits flags, int depth = -1);

  void setSessionId(const smtk::common::UUID& sessId);
  void setManager(Manager* mgr);

  virtual Entity* addEntityRecord(const EntityRef& entRef);
  virtual ArrangementHelper* createArrangementHelper();
  int findOrAddRelatedEntities(
    const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper);
  virtual int findOrAddCellAdjacencies(
    const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddCellUses(
    const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddOwningCell(
    const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddShellAdjacencies(
    const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddUseAdjacencies(
    const ShellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddGroupOwner(
    const Group& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddFreeCells(
    const Model& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedModels(
    const Model& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddPrototype(
    const Instance& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedModels(
    const SessionRef& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedGroups(
    const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedInstances(
    const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual SessionInfoBits findOrAddArrangements(
    const EntityRef& entRef, Entity* entRec, SessionInfoBits flags, ArrangementHelper* helper);
  virtual SessionInfoBits updateProperties(
    const EntityRef& entRef, Entity* entRec, SessionInfoBits flags, ArrangementHelper* helper);
  virtual SessionInfoBits updateTessellation(
    const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper);

#ifndef SHIBOKEN_SKIP
  void initializeOperatorSystem(const OperatorConstructors* opList);
  void importOperatorXML(const std::string& opXML);
  virtual OperatorConstructor findOperatorConstructorInternal(
    const std::string&, const OperatorConstructors* opList) const;
  virtual std::string findOperatorXMLInternal(
    const std::string&, const OperatorConstructors* opList) const;
#endif // SHIBOKEN_SKIP

  virtual SessionIOPtr createIODelegate(const std::string& format);

  DanglingEntities m_dangling;
  smtk::common::UUID m_sessionId;
  smtk::attribute::SystemPtr m_operatorSys;
  Manager* m_manager;
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_Session_h

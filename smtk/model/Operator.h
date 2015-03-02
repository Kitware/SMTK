//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Operator_h
#define __smtk_model_Operator_h
/*! \file */

//#include "smtk/Function.h" // for smtk::function<>
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SMTKCoreExports.h"
#include "smtk/AutoInit.h"

#include "smtk/model/Events.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/System.h"

#include <string>

namespace smtk {
  namespace io {
    class Logger;
  }
  namespace model {

#ifndef SHIBOKEN_SKIP
//typedef smtk::function<smtk::model::OperatorPtr()> OperatorConstructor;
typedef smtk::model::OperatorPtr (*OperatorConstructor)();
typedef std::pair<std::string,OperatorConstructor> StaticOperatorInfo;
typedef std::map<std::string,StaticOperatorInfo> OperatorConstructors;
#endif

/**\brief An enumeration of operation outcomes (or lacks thereof).
  *
  * These values are taken on by the "outcome" item of every OperatorResult.
  */
enum OperatorOutcome
{
  UNABLE_TO_OPERATE,   //!< The operator was misconfigured.
  OPERATION_CANCELED,  //!< An observer requested the operation be canceled. And it was.
  OPERATION_FAILED,    //!< The operator attempted to execute but encountered a problem.
  OPERATION_SUCCEEDED, //!< The operator succeeded.
  OUTCOME_UNKNOWN = -1 //!< The operator has not been run or the outcome is uninitialized.
};

/**\brief Boilerplate for classes that provide a solid modeling operator.
  *
  * Invoke this macro inside every class definition inheriting smtk::model::Operator.
  * Note that you must invoke this macro in a public section of your class declaration!
  *
  * You must also use the smtkImplementsModelOperator macro in your operator's implementation.
  */
#define smtkDeclareModelOperator() \
  static std::string operatorName; \
  virtual std::string name() const { return operatorName; } \
  virtual std::string className() const; \
  static smtk::model::OperatorPtr baseCreate();

/**\brief Declare that a class implements an operator for solid models.
  *
  * Invoke this macro inside every class definition inheriting smtk::model::Operator.
  * Several classes in smtk/cgm serve as examples.
  * Note that you must invoke this macro in the global namespace!
  *
  * You must also use the smtkDeclareModelOperator macro in your session's header.
  *
  * This macro takes 4 arguments:
  *
  * \a Cls       - The classname of your operator. This should be fully specified (i.e.,
  *                include namespaces).
  * \a Comp      - A "compilable" name for the operator. This is used as part of several function
  *                names, so it must be a valid variable name and should *not* be in quotes.
  *                It should *not* include namespaces and should be unique.
  * \a Nick      - A "short" name for the operator. This is meant as a text label for presentation
  *                name and *should* be in quotes.
  * \a ParamSpec - Either NULL or a string containing an XML description of the operator.
  *                The XML should contain an SMTK AttributeDefinition with the same name
  *                as this class's name() method returns. The smtk_
  * \a Brdg      - The name of the Session subclass to which this operator belongs.
  *                A pointer to the operator's create() method will be registered with the session
  *                during the dynamic variable initialization phase of this compilation unit
  *                (i.e., whenever the program or dynamic library containing this macro are
  *                loaded).
  */
#define smtkImplementsModelOperator(Cls, Comp, Nick, ParamSpec, Brdg) \
  /***\brief Adapt create() to return a base-class pointer (for register[Static]Operator). */ \
  smtk::model::OperatorPtr Cls ::baseCreate() { \
    return Cls ::create(); \
  } \
  /* Implement autoinit methods */ \
  void smtk_##Comp##_operator_AutoInit_Construct() { \
    Brdg ::registerStaticOperator( \
      Nick , /* Can't rely on operatorName to be initialized yet */ \
      ParamSpec, \
      Cls ::baseCreate); \
  } \
  void smtk_##Comp##_operator_AutoInit_Destruct() { \
    Brdg ::registerStaticOperator( \
      Cls ::operatorName, \
      NULL, \
      NULL); \
  } \
  /* Declare the component name */ \
  std::string Cls ::operatorName( Nick ); \
  /**\brief Provide a method to obtain the class name */ \
  std::string Cls ::className() const { return #Cls ; } \
  /* Force the registration methods above to be run on load */ \
  smtkComponentInitMacro(smtk_##Comp##_operator);

/**\brief A base class for solid modeling operations.
  *
  * Subclasses must use the smtkDeclareModelOperator macro
  * in their header file and smtkImplementsModelOperator macro
  * in their implementation file.
  * The latter accepts an XML description of the operator's
  * parameters and their default values.
  * A mechanism exists for generating a header file from an
  * XML file in the source tree, which is a convenient to
  * use the macro.
  *
  * Subclasses must also override the operateInternal() method and
  * may override the ableToOperate() method.
  *
  * Non-default parameter values are kept in an Attribute
  * instance that specifies the state of the Operator.
  *
  * Before and after operateInternal() is executed, the base class
  * inspects the model manager's Logger instance.
  * If any new records were created, they are serialized and
  * set as the result attribute's "log" item.
  * You may use ImportJSON to deserialize the log and present
  * it to users in your application.
  * This serialization is performed since SMTK operations are
  * often run in a remote process from the end-user application.
  *
  * Instances of the Operator class should always have a valid
  * pointer to their owning Session instance.
  * Every operator's specification() Attribute is managed by the
  * Session's operatorSystem().
  */
class SMTKCORE_EXPORT Operator : smtkEnableSharedPtr(Operator)
{
public:
  smtkTypeMacro(Operator);

  virtual std::string name() const = 0;
  virtual std::string className() const = 0;
  virtual bool ableToOperate();
  virtual OperatorResult operate();

#ifndef SHIBOKEN_SKIP
  void observe(OperatorEventType event, WillOperateCallback functionHandle, void* callData);
  void observe(OperatorEventType event, DidOperateCallback functionHandle, void* callData);

  void unobserve(OperatorEventType event, WillOperateCallback functionHandle, void* callData);
  void unobserve(OperatorEventType event, DidOperateCallback functionHandle, void* callData);

  int trigger(OperatorEventType event);
  int trigger(OperatorEventType event, const OperatorResult& result);
#endif // SHIBOKEN_SKIP

  ManagerPtr manager() const;
  Ptr setManager(ManagerPtr s);

  Session* session() const;
  Ptr setSession(Session* b);

  smtk::io::Logger& log();

  OperatorDefinition definition() const;

  smtk::attribute::AttributePtr specification() const;
  bool setSpecification(smtk::attribute::AttributePtr spec);
  bool ensureSpecification() const;

  /// Convenience method for finding a operator parameter of a known type.
  template<typename T> typename T::Ptr findAs(
    const std::string& pname,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN)
    { return this->specification()->findAs<T>(pname, style); }

  smtk::attribute::IntItemPtr findInt(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::DoubleItemPtr findDouble(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::StringItemPtr findString(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::FileItemPtr findFile(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::DirectoryItemPtr findDirectory(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::GroupItemPtr findGroup(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::RefItemPtr findRef(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::ModelEntityItemPtr findModelEntity(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);
  smtk::attribute::MeshSelectionItemPtr findMeshEntity(
    const std::string& name,
    smtk::attribute::SearchStyle style = smtk::attribute::ALL_CHILDREN);

  bool associateEntity(const smtk::model::EntityRef& entity);
  void disassociateEntity(const smtk::model::EntityRef& entity);
  void removeAllAssociations();
  template<typename T> T associatedEntitiesAs() const;

  OperatorResult createResult(OperatorOutcome outcome = UNABLE_TO_OPERATE);
  void eraseResult(OperatorResult res);

#ifndef SHIBOKEN_SKIP
  bool operator < (const Operator& other) const;
#endif // SHIBOKEN_SKIP

protected:
  friend class DefaultSession;

  Operator();
  virtual ~Operator();

  virtual OperatorResult operateInternal() = 0;

#ifndef SHIBOKEN_SKIP
  ManagerPtr m_manager; // Model manager, not the attribute manager for the operator.
  Session* m_session;
  OperatorSpecification m_specification;
  std::set<WillOperateObserver> m_willOperateTriggers;
  std::set<DidOperateObserver> m_didOperateTriggers;
#endif // SHIBOKEN_SKIP
};

SMTKCORE_EXPORT std::string outcomeAsString(int oc);
SMTKCORE_EXPORT OperatorOutcome stringToOutcome(const std::string& oc);


template<typename T>
T Operator::associatedEntitiesAs() const
{
  bool resetMgr = false;
  this->ensureSpecification();
  if (this->m_specification->system())
    if (!this->m_specification->modelManager())
      {
      resetMgr = true;
      this->m_specification->system()->setRefModelManager(this->m_manager);
      }
  T result = this->m_specification->associatedModelEntities<T>();
  if (resetMgr)
    this->m_specification->system()->setRefModelManager(smtk::model::ManagerPtr());
  return result;
}

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Operator_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_Operator_h
#define __smtk_operation_Operator_h

#include "smtk/AutoInit.h"
#include "smtk/CoreExports.h"
#include "smtk/Function.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include <string>
#include <typeindex>
#include <utility>

namespace smtk
{
namespace attribute
{
class Attribute;
}
namespace io
{
class Logger;
}
namespace operation
{

/**\brief Boilerplate for operations.
  *
  * Invoke this macro inside every class definition inheriting smtk::operation::Operator.
  * Note that you must invoke this macro in a public section of your class declaration!
  *
  * You must also use the smtkImplementsOperator macro in your operator's implementation.
  */
#define smtkDeclareOperator()                                                                      \
  static std::string operatorName;                                                                 \
  std::string name() const override { return operatorName; }                                       \
  std::string className() const override

/**\brief Declare that a class implements an operator.
  *
  * Invoke this macro inside every class definition inheriting smtk::operation::Operator.
  * Note that you must invoke this macro in the global namespace!
  *
  * You must also use the smtkDeclareOperator macro in your session's header.
  *
  * This macro takes 6 arguments:
  *
  * \a ExportSym - The symbol used to export the AutoInit functions.
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
  */
#define smtkImplementsOperator(ExportSym, Cls, Comp, Nick, ParamSpec)                              \
  /* Implement autoinit methods */                                                                 \
  void ExportSym smtk_##Comp##_operator_AutoInit_Construct()                                       \
  {                                                                                                \
    smtk::operation::Manager::registerStaticOperator<Cls>(                                         \
      #Cls, Nick, ParamSpec, []() -> smtk::operation::OperatorPtr {                                \
        return std::static_pointer_cast<smtk::operation::Operator>(Cls::create());                 \
      });                                                                                          \
  }                                                                                                \
  void ExportSym smtk_##Comp##_operator_AutoInit_Destruct()                                        \
  {                                                                                                \
    smtk::operation::Manager::unregisterStaticOperator<Cls>();                                     \
  }                                                                                                \
  /* Declare the component name */                                                                 \
  std::string Cls::operatorName(Nick);                                                             \
  /**\brief Provide a method to obtain the class name */                                           \
  std::string Cls::className() const { return #Cls; }                                              \
  /* Force the registration methods above to be run on load */                                     \
  smtkComponentInitMacro(smtk_##Comp##_operator);

/**\brief A base class for operations.
  *
  * Subclasses must use the smtkDeclareOperator macro
  * in their header file and smtkImplementsOperator macro
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
  * inspects the Logger instance.
  * If any new records were created, they are serialized and
  * set as the result attribute's "log" item.
  * You may use LoadJSON to deserialize the log and present
  * it to users in your application.
  * This serialization is performed since SMTK operations are
  * often run in a remote process from the end-user application.
  */
class SMTKCORE_EXPORT Operator : smtkEnableSharedPtr(Operator)
{
public:
  typedef std::type_index Index;
  typedef smtk::function<smtk::operation::OperatorPtr()> Constructor;

  struct Info
  {
    Info(const std::string& clsname, const std::string& nkname, const std::string& xmlDesc,
      const Operator::Constructor& opConstructor)
      : classname(clsname)
      , nickname(nkname)
      , xml(xmlDesc)
      , constructor(opConstructor)
    {
    }

    std::string classname;
    std::string nickname;
    std::string xml;
    Operator::Constructor constructor;
  };

  typedef smtk::shared_ptr<smtk::attribute::Attribute> Result;
  typedef smtk::shared_ptr<smtk::attribute::Definition> Definition;
  typedef smtk::shared_ptr<smtk::attribute::Attribute> Specification;

  /**\brief Enumerate events that an operator may encounter.
 *
 * No event is provided for operator deletion because
 * (1) operator deletion is not managed and
 * (2) "this" is not complete in class destructors (subclass data is already
 * freed).
 * So, there is no easy way to observe when an operator is about to be
 * deleted but is still valid.
 */
  enum EventType
  {
    CREATED_OPERATOR, //!< An instance of the Operator class has been created.
    WILL_OPERATE,     //!< The operation will commence if no observers cancel it.
    DID_OPERATE       //!< The operation has completed or been canceled.
  };

  /**\brief An enumeration of operation outcomes (or lacks thereof).
 *
 * These values are taken on by the "outcome" item of every OperatorResult.
 */
  enum Outcome
  {
    UNABLE_TO_OPERATE,   //!< The operator was misconfigured.
    OPERATION_CANCELED,  //!< An observer requested the operation be canceled. And it was.
    OPERATION_FAILED,    //!< The operator attempted to execute but encountered a problem.
    OPERATION_SUCCEEDED, //!< The operator succeeded.
    OUTCOME_UNKNOWN = -1 //!< The operator has not been run or the outcome is uninitialized.
  };

  // Callbacks for CREATED_OPERATOR and WILL_OPERATE events provide access to the operator.
  // Returning non-zero values cancel the operation.
  typedef int (*Callback)(EventType event, const Operator& op, void* user);

  // An observer of CREATED_OPERATOR or WILL_OPERATE events binds a callback and opaque,
  // user-provided data.
  typedef std::pair<Callback, void*> Observer;

  // A trigger for CREATED_OPERATOR or WILL_OPERATE events holds the event type and its observer.
  typedef std::pair<EventType, Observer> Trigger;

  // Callbacks for DID_OPERATE events provide access to the operator and the results of the
  // operation. Return values are ignored.
  typedef int (*CallbackWithResult)(
    EventType event, const Operator& op, Operator::Result r, void* user);

  // An observer of DID_OPERATE events binds a callback and opaque, user-provided data.
  typedef std::pair<CallbackWithResult, void*> ObserverWithResult;

  smtkTypeMacroBase(Operator);

  virtual std::string name() const = 0;
  virtual std::string className() const = 0;
  virtual bool ableToOperate();
  virtual Result operate();

  /// Return the type of the operator.
  Index index() const { return typeid(*this); }

  void observe(EventType event, Callback functionHandle, void* callData);
  void observe(EventType event, CallbackWithResult functionHandle, void* callData);

  void unobserve(EventType event, Callback functionHandle, void* callData);
  void unobserve(EventType event, CallbackWithResult functionHandle, void* callData);

  int trigger(EventType event);
  int trigger(EventType event, const Operator::Result& result);

  virtual smtk::io::Logger& log() = 0;

  virtual Definition definition() const;
  virtual Specification specification() const;
  virtual bool setSpecification(Specification spec);
  virtual bool ensureSpecification() const;

  virtual Result createResult(Outcome outcome = UNABLE_TO_OPERATE);
  void setResultOutcome(Result res, Outcome outcome);
  virtual void eraseResult(Result res);

  void setManager(ManagerPtr mgr);

  bool operator<(const Operator& other) const;

  virtual ~Operator();

protected:
  Operator();

  /// Perform the operation. Subclasses must implement this method.
  virtual Operator::Result operateInternal() = 0;
  /** \brief Post-process result entities. Called from within operate(), after operateInternal().
    *
    * Perform tasks that affect only the SMTK model manager's storage.
    *
    * This method mirrors created/expunged/modified/tess_changed model entity items to
    * component items as a temporary measure until operators move away from using
    * ModelEntityItem.
    */
  virtual void postProcessResult(Operator::Result& res);

  void copyModelEntityItemToComponentItem(
    Operator::Result& res, const std::string& meItem, const std::string& compItem);

  /// Add a summary log-entry of the result. Called from within operate(), after prettifyResult().
  virtual void generateSummary(Operator::Result& res);

  Specification m_specification;
  std::set<Observer> m_willOperateTriggers;
  std::set<ObserverWithResult> m_didOperateTriggers;
  int m_debugLevel;
  std::weak_ptr<Manager> m_manager;
};

SMTKCORE_EXPORT std::string outcomeAsString(int oc);
SMTKCORE_EXPORT Operator::Outcome stringToOutcome(const std::string& oc);
}
}

#endif // __smtk_model_Operator_h

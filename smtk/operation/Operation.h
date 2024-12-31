//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_Operation_h
#define smtk_operation_Operation_h

#include "smtk/resource/Lock.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/common/Deprecation.h"

#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <utility>

namespace smtk
{
namespace attribute
{
class Attribute;
class Resource;
} // namespace attribute
namespace io
{
class Logger;
}
namespace operation
{
class Helper;

class ImportPythonOperation;
class Manager;
class Operation;

using Handler = std::function<void(Operation&, const std::shared_ptr<smtk::attribute::Attribute>&)>;

/// Hold a set of resources to be locked for an operation along with the type of lock to acquire.
using ResourceAccessMap = std::map<
  std::weak_ptr<smtk::resource::Resource>,
  smtk::resource::LockType,
  std::owner_less<std::weak_ptr<smtk::resource::Resource>>>;

/// Operation is a base class for all SMTK operations. SMTK operations are
/// essentially functors that operate on SMTK resources and resource components.
/// Their input parameters and output results are described by an SMTK attribute
/// resource that is constructed in derived operation classes via the method
/// createSpecification(). There should be exactly one attribute definition that
/// derives from "operation" and one definition that derives from "result". The
/// creation, destruction and manipulation of SMTK resources and components
/// occurs in the method operateInternal(). Operations are designed to work in
/// conjunction with an smtk::operation::Manager, but can also be used as
/// standalone functors.
class SMTKCORE_EXPORT Operation : smtkEnableSharedPtr(Operation)
{
  friend class Helper;

public:
  smtkTypeMacroBase(smtk::operation::Operation);

  // A hash value uniquely representing the operation.
  typedef std::size_t Index;

  // An attribute describing the operation's input.
  typedef std::shared_ptr<smtk::attribute::Attribute> Parameters;

  // An attribute containing the operation's result.
  typedef std::shared_ptr<smtk::attribute::Attribute> Result;

  // An attribute resource containing the operation's execution definition
  // result definition.
  typedef std::shared_ptr<smtk::attribute::Resource> Specification;

  typedef std::shared_ptr<smtk::attribute::Definition> Definition;

  // These values are taken on by the "outcome" item of every Operation Result.
  enum class Outcome
  {
    UNABLE_TO_OPERATE, //!< The operation is misconfigured.
    CANCELED,          //!< An observer requested the operation be canceled. And it was.
    FAILED,            //!< The operation attempted to execute but encountered a problem.
    SUCCEEDED,         //!< The operation succeeded.
    UNKNOWN = -1       //!< The operation has not been run or the outcome is uninitialized.
  };

  friend Manager;
  friend ImportPythonOperation;

  virtual ~Operation();

  // Index is a compile-time intrinsic of the derived operation; as such, it
  // cannot be set. It is virtual so that derived operations can assign their
  // own index (as is necessary for python operations that would otherwise all
  // resolve to the same index).
  virtual Index index() const { return std::type_index(typeid(*this)).hash_code(); }

  /// Update the operation's specification and operations to be consistent.
  ///
  /// This does nothing by default but subclasses may override this method to
  /// update default values (say, based on the current set of associations).
  /// This method should be called by user interfaces when the associations
  /// (and potentially other attributes/items in the specification) are modified.
  ///
  /// By default, the attribute and item passed are null.
  /// If values are passed, the attribute or item **must** belong to the
  /// resource for the operation itself, and should indicate changes made
  /// by the user.
  /// When an attribute is created or removed at the user's behest it is passed.
  /// When an item's value(s) are directly edited by a user, then
  /// a pointer to it is passed.
  /// Only one value (the item or the attribute) should be non-null
  /// for a given invocation of configure().
  /// Both may be null (for instance, when an operation is being asked to
  /// initialize its parameters based on the global application state rather
  /// than a particular user input).
  ///
  /// This method should return true if any changes were made to the operation's
  /// specification and false otherwise.
  virtual bool configure(
    const smtk::attribute::AttributePtr& changedAttribute = smtk::attribute::AttributePtr(),
    const smtk::attribute::ItemPtr& changedItem = smtk::attribute::ItemPtr());

  /// Check if the operation's attribute resource is valid. Derived operations
  /// may implement more task-specific checks to ensure that the operation is in
  /// a valid state.
  virtual bool ableToOperate();

  /// Execute the operation, log its outcome and return its results. This method
  /// calls operateInternal() and handles additional bookkeeping.
  Result operate();

  /// Execute the operation in an asynchronous environment. This method does not
  /// return until the operation is complete, but because it does not return a
  /// result, only observers can process the operation. This prevents simultaneous,
  /// unguarded access to results as observers are invoked before resource locks
  /// are released.
  ///
  /// If you pass a \a handler to safeOperate, it will be invoked after other
  /// observers have run but before resource locks have been released (thus
  /// preventing deadlocks or unguarded access to the result).
  ///
  /// Unlike Operation::operate(), this method removes its Result from the specification
  /// before returning.
  ///
  /// This method will eventually replace Operation::operate().
  Outcome safeOperate();
  Outcome safeOperate(Handler handler);

  /// Release the operation \a result returned by `operate()`.
  /// Note that if you do not release the result, it will eventually
  /// be released when the operation itself is destroyed.
  ///
  /// This removes the Result (an smtk::attribute::Attribute) from the operation's
  /// resource; anyone holding the shared pointer to the result will keep the
  /// attribute in memory but will experience inconsistent behavior since its items
  /// are removed as part of releasing it from control by the attribute::Resource.
  virtual bool releaseResult(Result& result);

  /// Retrieve the operation's logger. By default, we use the singleton logger.
  /// Derived classes can reimplement this method if an alternative logging
  /// system is needed.
  virtual smtk::io::Logger& log() const;

  /// This accessor facilitates the lazy construction of the specification,
  /// allowing for derived implementations of its creation. More sophisticated
  /// operations may contain additional attributes as input parameters; they can
  /// be accessed through the specification.
  Specification specification();

  /// Access the operation's input parameters, constructing them if necessary.
  /// The parameters attribute is distinguished by its derivation from the
  /// "operation" attribute.
  Parameters parameters();
  Parameters parameters() const;

  /// Create an attribute representing this operation's result type. The result
  /// attribute is distinguished by its derivation from the "result" attribute.
  Result createResult(Outcome);

  /// Operations that are managed have a non-null pointer to their manager.
  ManagerPtr manager() const { return m_manager.lock(); }

  /// restore operation parameters from the trace of a previously run operation.
  bool restoreTrace(const std::string& trace);

  /// Operations may be passed application state in the form of a Managers type-container.
  void setManagers(const std::shared_ptr<smtk::common::Managers>& m) { m_managers = m; }
  std::shared_ptr<smtk::common::Managers> managers() const { return m_managers; }

  /// Is this type of operation safe to launch in a thread?
  virtual bool threadSafe() const { return true; }

  /// retrieve the resource manager, if available.
  smtk::resource::ManagerPtr resourceManager();

  /// When running a nested operation, specify how resource locks should be handled.
  enum LockOption
  {
    LockAll,         //!< Lock all resources mentioned in the nested operation's parameters.
    ParentLocksOnly, //!< Use only the locks already held by this (the parent's) operation.
    SkipLocks        //!< Do not attempt to lock any resources.
  };

  /// When running a nested operation, specify whether observers should be invoked.
  ///
  /// This does not control the outermost operation's observers; those will always
  /// be invoked.
  enum ObserverOption
  {
    InvokeObservers, //!< Do not invoke observers for the internal/nested operation.
    SkipObservers    //!< Skip running observers for the internal/nested operation.
  };

  /// When running a nested operation, specify whether to call the
  /// operation's ableToOperate() method or skip it.
  enum ParametersOption
  {
    Validate,      //!< Ensure the nested operation's parameters are valid.
    SkipValidation //!< Assume the nested operation's parameters are valid.
  };

protected:
  Operation();

  /// Identify resources to lock, and whether to lock them for reading or writing.
  ///
  /// The default implementation simply calls extractResourceAndLockTypes()
  /// on the operation's parameters.
  virtual ResourceAccessMap identifyLocksRequired();

  /// Returns the set of resources that are currently locked by this operation.
  const ResourceAccessMap& lockedResources() const { return this->m_lockedResources; }

  /// Perform the actual operation and construct the result.
  virtual Result operateInternal() = 0;

  // Apply post-processing to the result object. This method should not modify
  // the modeling kernel but may change string/float/int properties stored on
  // entities.
  virtual void postProcessResult(Result&) {}

  // Mark resources as dirty or clean according to their use in the operation.
  // By default, all resources used as inputs with Write LockTypes and all
  // resources referenced in the result are marked dirty.
  virtual void markModifiedResources(Result&);

  // Remove resources from the resource manager.
  virtual bool unmanageResources(Result&);

  // Append an output summary string to the output result. Derived classes can
  // reimplement this method to send custom summary strings to the logger.
  virtual void generateSummary(Result&);

  // Construct the operation's base specification. This is done by reading
  // an attribute .sbt file.
  Specification createBaseSpecification() const;

  int m_debugLevel{ 0 };
  std::weak_ptr<Manager> m_manager;
  std::shared_ptr<smtk::common::Managers> m_managers;

  /// A key used to run operations nested from within this operation.
  ///
  /// The key cannot be constructed outside of classes that inherit
  /// operations, so that developers cannot accidentally run operations
  /// with the proper resource locking.
  struct BaseKey
  {
    BaseKey() = default;
    BaseKey(
      const Operation* parent,
      ObserverOption observerOption,
      LockOption lockOption,
      ParametersOption paramsOption)
      : m_parent(parent)
      , m_lockOption(lockOption)
      , m_observerOption(observerOption)
      , m_paramsOption(paramsOption)
    {
    }

    const Operation* m_parent{ nullptr };
    LockOption m_lockOption{ LockOption::SkipLocks };
    ObserverOption m_observerOption{ ObserverOption::SkipObservers };
    ParametersOption m_paramsOption{ ParametersOption::SkipValidation };
  };

  struct SMTK_DEPRECATED_IN_24_11("Use this->childKey() instead.") Key : BaseKey
  {
    Key() = default;
  };

  /// Return a key that will allow subclasses of Operation to run other
  /// nested operations (from within `operateInternal()`).
  ///
  /// The options passed to this method control how the nested operation
  /// is validated before running and whether to invoke observers on
  /// the sub-operation or not.
  BaseKey childKey(
    ObserverOption observerOption = ObserverOption::SkipObservers,
    LockOption lockOption = LockOption::LockAll,
    ParametersOption paramsOption = ParametersOption::Validate) const;

public:
  /// Run an operation given a key returned by Operation::childKey().
  Result operate(const BaseKey& key);

private:
  // Construct the operation's specification. This is typically done by reading
  // an attribute .sbt file, but can be done by first constructing a base
  // specification and then augmenting the specification to include the derived
  // operation's input and output attributes.
  virtual Specification createSpecification() = 0;

  void unlockResources(const ResourceAccessMap& resources);

  Specification m_specification;
  Parameters m_parameters;
  Definition m_resultDefinition;
  std::vector<std::weak_ptr<smtk::attribute::Attribute>> m_results;
  ResourceAccessMap m_lockedResources;
};

/**\brief Return the outcome of an operation given its \a result object.
  *
  * This saves the tedium of fetching the integer item and casting it
  * to an outcome.
  */
SMTKCORE_EXPORT Operation::Outcome outcome(const Operation::Result& result);
SMTKCORE_EXPORT bool setOutcome(const Operation::Result& result, Operation::Outcome outcome);

} // namespace operation
} // namespace smtk

#endif // smtk_operation_Operation_h

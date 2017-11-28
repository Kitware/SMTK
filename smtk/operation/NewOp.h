//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_NewOp_h
#define __smtk_operation_NewOp_h

#include "smtk/CoreExports.h"
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
class Collection;
}
namespace io
{
class Logger;
}
namespace operation
{
class ImportPythonOperator;
class Manager;
class Metadata;

/// Operator is a base class for all SMTK operators. SMTK operators are
/// essentially functors that operate on SMTK resources and resource components.
/// Their input parameters and output results are described by an SMTK attribute
/// resource that is constructed in derived operator classes via the method
/// createSpecification(). There should be exactly one attribute definition that
/// derives from "operator" and one definition that derives from "result". The
/// creation, destruction and manipulation of SMTK resources and components
/// occurs in the method operateInternal(). Operators are designed to work in
/// conjunction with an smtk::operation::Manager, but can also be used as
/// standalone functors.
class SMTKCORE_EXPORT NewOp : smtkEnableSharedPtr(NewOp)
{
public:
  smtkTypeMacroBase(NewOp);

  // A hash value uniquely representing the operator.
  typedef std::size_t Index;

  // An attribute describing the operator's input.
  typedef std::shared_ptr<smtk::attribute::Attribute> Parameters;

  // An attribute containing the operator's result.
  typedef std::shared_ptr<smtk::attribute::Attribute> Result;

  // An attribute collection containing the operator's execution definition
  // result definition.
  typedef std::shared_ptr<smtk::attribute::Collection> Specification;

  // These values are taken on by the "outcome" item of every Operator Result.
  enum class Outcome
  {
    UNABLE_TO_OPERATE, //!< The operator is misconfigured.
    CANCELED,          //!< An observer requested the operation be canceled. And it was.
    FAILED,            //!< The operator attempted to execute but encountered a problem.
    SUCCEEDED,         //!< The operator succeeded.
    UNKNOWN = -1       //!< The operator has not been run or the outcome is uninitialized.
  };

  friend class Manager;
  friend ImportPythonOperator;

  // Return a unique string that describes this class. Registered operators
  // return their registered unique name; unregistered operators simply return
  // this->classname().
  std::string uniqueName() const;

  // Index is a compile-time intrinsic of the derived operator; as such, it
  // cannot be set. It is virtual so that derived operators can assign their own
  // index (as is necessary for python operators that would otherwise all
  // resolve to the same index).
  virtual Index index() const { return std::type_index(typeid(*this)).hash_code(); }

  // Check if the operator's attribute collection is valid. Derived operators
  // may implement more task-specific checks to ensure that the operator is in a
  // valid state.
  virtual bool ableToOperate();

  // Execute the operation, log its outcome and return its results. This method
  // calls operateInternal() and handles additional bookkeeping.
  Result operate();

  // Retrieve the operator's logger. By default, we use the singleton logger.
  // Derived classes can reimplement this method if an alternative logging
  // system is needed.
  virtual smtk::io::Logger& log() const;

  // This accessor facilitates the lazy construction of the specification,
  // allowing for derived implementations of its creation. More sophisticated
  // operators may contain additional attributes as input parameters; they can
  // be accessed through the specification.
  Specification specification();

  // Access the operator's input parameters, constructing them if necessary. The
  // parameters attribute is distinguished by its derivation from the "operator"
  // attribute.
  Parameters parameters();

  // Create an attribute representing this operator's result type. The result
  // attribute is distinguished by its derivation from the "result" attribute.
  Result createResult(Outcome);

  virtual ~NewOp();

protected:
  NewOp();

  // Perform the actual operation and construct the result.
  virtual Result operateInternal() = 0;

  // Apply post-processing to the result object. This method should not modify
  // the modeling kernel but may change string/float/int properties stored on
  // entities.
  virtual void postProcessResult(Result&) {}

  // Append an output summary string to the output result. Derived classes can
  // reimplement this method to send custom summary strings to the logger.
  virtual void generateSummary(Result&);

  // Construct the operator's base specification. This is done by reading
  // an attribute .sbt file.
  Specification createBaseSpecification() const;

  int m_debugLevel;

private:
  typedef std::shared_ptr<smtk::attribute::Definition> Definition;

  // Construct the operator's specification. This is typically done by reading
  // an attribute .sbt file, but can be done by first constructing a base
  // specification and then augmenting the specification to include the derived
  // operator's input and output attributes.
  virtual Specification createSpecification() = 0;

  Specification m_specification;
  Parameters m_parameters;
  Definition m_resultDefinition;
  std::vector<Result> m_results;
  Manager* m_manager;
};
}
}

#endif // __smtk_operation_NewOp_h

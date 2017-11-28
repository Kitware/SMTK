//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_model_PyOperator_h
#define smtk_model_PyOperator_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <type_traits>

#include "smtk/attribute/Collection.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/NewOp.h"

#include "smtk/common/PythonInterpreter.h"

namespace smtk
{
namespace operation
{
class PyOperator : public NewOp
{
public:
  PyOperator() : NewOp() {}
  virtual ~PyOperator() {}

  static std::shared_ptr<smtk::operation::NewOp> create(std::string modulename, std::string classname, smtk::operation::NewOp::Index index)
    {
      // Import the module containing our operator
      pybind11::module module = pybind11::module::import(modulename.c_str());

      // Create an instance of our operator
      pybind11::object obj = module.attr(classname.c_str())();

      if (smtk::common::PythonInterpreter::instance().isEmbedded())
      {
        // If we are running in embedded mode, we allow the C++ side to perform
        // memory management. We do this by internally incrementing the
        // python-side object's internel reference count to ensure that the
        // python environment does not delete our operator out from under us.
        obj.inc_ref();

        std::shared_ptr<smtk::operation::PyOperator> op(obj.cast<smtk::operation::PyOperator*>());
        // Have the instance hold onto its python object, so when the instance
        // is removed so is the tether to the python object.
        op->setObject(obj);

        // For C++ operators, index() is a compile-time intrinsic of the
        // operator class. Python operators only come into existence at runtime,
        // though, so we need to manually set a python operator's index.
        op->setIndex(index);

        return std::static_pointer_cast<smtk::operation::NewOp>(op);
      }
      else
      {
        // Have the instance hold onto its python object, so when the instance
        // is removed so is the tether to the python object.
        obj.cast<std::shared_ptr<smtk::operation::PyOperator> >()->setObject(obj);

        // For C++ operators, index() is a compile-time intrinsic of the
        // operator class. Python operators only come into existence at runtime,
        // though, so we need to manually set a python operator's index.
        obj.cast<std::shared_ptr<smtk::operation::PyOperator> >()->setIndex(index);

        // If we are running in a native python instance (i.e. not our embedded
        // instance), then memory management is handled by python. We need only
        // to cast our python object into a shared_ptr so the SMTK operator
        // handling system can use it.
        return obj.cast<std::shared_ptr<smtk::operation::NewOp> >();
      }
    }

  Index index() const override { return m_index; }

  bool ableToOperate() override { PYBIND11_OVERLOAD(bool, NewOp, ableToOperate, ); }

  smtk::io::Logger& log() const override { PYBIND11_OVERLOAD(smtk::io::Logger&, NewOp, log, ); }

  Result operateInternal() override { PYBIND11_OVERLOAD_PURE(Result, NewOp, operateInternal, ); }

  void postProcessResult(Result& res) override
    { PYBIND11_OVERLOAD(void, NewOp, postProcessResult, res); }

  void generateSummary(Result& res) override
    { PYBIND11_OVERLOAD(void, NewOp, generateSummary, res); }

  // We incorporate the base class's method with a different access modifier
  using NewOp::createBaseSpecification;

private:
  Specification createSpecification() override
    { PYBIND11_OVERLOAD_PURE(Specification, NewOp, createSpecification); }

  void setObject(pybind11::object obj) { m_object = obj; }
  void setIndex(Index index) { m_index = index; }

  pybind11::object m_object;
  Index m_index;
};
}
}

#endif

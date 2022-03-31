//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_model_PyOperation_h
#define smtk_model_PyOperation_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <type_traits>

#include "smtk/io/Logger.h"

#include "smtk/operation/Operation.h"

#include "smtk/common/PythonInterpreter.h"

namespace pybind11
{
namespace detail
{

template <>
struct type_caster<std::shared_ptr<smtk::operation::Operation> >
{
  PYBIND11_TYPE_CASTER(std::shared_ptr<smtk::operation::Operation>, _("Operation"));

  using OperationCaster = copyable_holder_caster<smtk::operation::Operation,
                                                 std::shared_ptr<smtk::operation::Operation> >;

  bool load(handle src, bool b)
  {
    OperationCaster oc;
    bool success = oc.load(src, b);
    if (!success)
    {
      return false;
    }

    auto py_obj = reinterpret_borrow<object> (src);
    auto base_ptr = static_cast<std::shared_ptr<smtk::operation::Operation> > (oc);

    // Construct a shared_ptr to the object
    auto py_obj_ptr = std::shared_ptr<object>{
        new object{py_obj},
        [](object* py_object_ptr)
        {
          // It's possible that when the shared_ptr dies we won't have the
          // gil (if the last holder is in a non-Python thread), so we
          // make sure to acquire it in the deleter.
          gil_scoped_acquire gil;
          delete py_object_ptr;
        }
    };

    value = std::shared_ptr<smtk::operation::Operation> (py_obj_ptr, base_ptr.get());
    return true;
  }

  static handle cast (std::shared_ptr<smtk::operation::Operation> base,
                      return_value_policy rvp,
                      handle h)
  {
    return OperationCaster::cast (base, rvp, h);
  }
};

template <>
struct is_holder_type<smtk::operation::Operation,
                      std::shared_ptr<smtk::operation::Operation> > : std::true_type {};
}
}

namespace smtk
{
namespace operation
{
class PyOperation : public Operation
{
public:
  PyOperation() = default;
  ~PyOperation() override = default;

  static std::shared_ptr<smtk::operation::Operation> create(std::string modulename,
                                                            std::string className,
                                                            smtk::operation::Operation::Index index)
    {
      // Import the module containing our operation
      pybind11::module module = pybind11::module::import(modulename.c_str());

      // Create an instance of our operation
      pybind11::object obj = module.attr(className.c_str())();

      // For C++ operations, index() is a compile-time intrinsic of the
      // operation class. Python operations only come into existence at runtime,
      // though, so we need to manually set a python operation's index.
      obj.cast<std::shared_ptr<smtk::operation::PyOperation> >()->setIndex(index);

      // The precedent for python operation names is estabilished in
      // ImportPythonOperation to be the modulename.className
      // We follow that convention here.
      obj.cast<std::shared_ptr<smtk::operation::PyOperation> >()->setTypeName(
        modulename + "." + className);

      return obj.cast<std::shared_ptr<smtk::operation::Operation> >();
    }

  Index index() const override { return m_index; }

  bool ableToOperate() override
    {
      // When a python operation is constructed on one thread and called on
      // another, we need to temporarily cut the relationship between
      // PyThreadState and the original thread before calling this method on a
      // new thread. Otherwise, we result in deadlock. Pybind11's
      // gil_scoped_release provides this functionality.
      pybind11::gil_scoped_release thread_state(true);
      PYBIND11_OVERLOAD(bool, Operation, ableToOperate, );
    }

  std::string typeName() const override { return m_typeName; }

  smtk::io::Logger& log() const override
    {
      // When a python operation is constructed on one thread and called on
      // another, we need to temporarily cut the relationship between
      // PyThreadState and the original thread before calling this method on a
      // new thread. Otherwise, we result in deadlock. Pybind11's
      // gil_scoped_release provides this functionality.
      pybind11::gil_scoped_release thread_state(true);
      PYBIND11_OVERLOAD(smtk::io::Logger&, Operation, log, );
    }

  Result operateInternal() override
    {
      // Python operations often raise exceptions. If they are uncaught, they
      // become C++ exceptions. We convert these exceptions to a failed
      // execution.
      try
      {
        return this->operateInternalPy();
      }
      catch(std::exception& e)
      {
        this->log().addRecord(smtk::io::Logger::Error, e.what());
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }

  void postProcessResult(Result& res) override
    { PYBIND11_OVERLOAD(void, Operation, postProcessResult, res); }

  void generateSummary(Result& res) override
    { PYBIND11_OVERLOAD(void, Operation, generateSummary, res); }

  // We incorporate the base class's method with a different access modifier
  using Operation::createBaseSpecification;

  /// Avoid PyGILState_Check() failures by running on the main thread by default.
  bool threadSafe() const override { return false; }

private:
  Specification createSpecification() override
    { PYBIND11_OVERLOAD_PURE(Specification, Operation, createSpecification); }

  void setIndex(Index index) { m_index = index; }
  void setTypeName(const std::string& typeName) { m_typeName = typeName; }

  Result operateInternalPy() { PYBIND11_OVERLOAD_PURE(Result, Operation, operateInternal, ); }

  Index m_index;
  std::string m_typeName;
};
}
}

#endif

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
class SMTKCORE_EXPORT PyOperation : public Operation
{
public:
  PyOperation() = default;
  ~PyOperation() override = default;

  using SimpleFunction = std::function<void(void)>;

  /// This is a functor that multi-threaded applications should provide
  /// in order to force a SimpleFunction to be run on the main thread.
  /// The default implementation simply assumes it will always be invoked
  /// on the main thread and directly invokes the function it is passed.
  static std::function<void(SimpleFunction)> runOnMainThread;

  static std::shared_ptr<smtk::operation::Operation> create(std::string modulename,
                                                            std::string className,
                                                            smtk::operation::Operation::Index index)
    {
      std::shared_ptr<smtk::operation::Operation> result;

      PyOperation::runOnMainThread([&result, &modulename, &className, &index]()
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

          result = obj.cast<std::shared_ptr<smtk::operation::Operation> >();
        }
      );
      return result;
    }

  Index index() const override { return m_index; }

  bool ableToOperate() override
    {
      bool able;
      PyOperation::runOnMainThread([&]()
        {
          able = this->ableToOperateMainThread();
        }
      );
      return able;
    }

  bool ableToOperateMainThread()
  {
    PYBIND11_OVERLOAD(bool, Operation, ableToOperate, );
  }

  std::string typeName() const override { return m_typeName; }

  smtk::io::Logger& log() const override
    {
      smtk::io::Logger* result;
      PyOperation::runOnMainThread([&]()
        {
          result = &(this->logMainThread());
        }
      );
      return *result;
    }

  smtk::io::Logger& logMainThread() const
  {
    PYBIND11_OVERLOAD(smtk::io::Logger&, Operation, log, );
  }

  Result operateInternal() override
    {
      Result result = nullptr;
      PyOperation::runOnMainThread([&]()
        {
          // Python operations often raise exceptions. If they are uncaught, they
          // become C++ exceptions. We convert these exceptions to a failed
          // execution.
          try
          {
            result = this->operateInternalPy();
          }
          catch(std::exception& e)
          {
            // We call logMainThread() here since we are already on the main thread:
            this->logMainThread().addRecord(smtk::io::Logger::Error, e.what());
            result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
          }
        }
      );
      return result;
    }

  void postProcessResult(Result& res) override
  {
    PyOperation::runOnMainThread([&]()
      {
        this->postProcessResultMainThread(res);
      }
    );
  }
  void postProcessResultMainThread(Result& res)
    { PYBIND11_OVERLOAD(void, Operation, postProcessResult, res); }

  void generateSummary(Result& res) override
  {
    PyOperation::runOnMainThread([&]()
      {
        this->generateSummaryMainThread(res);
      }
    );
  }
  void generateSummaryMainThread(Result& res)
    { PYBIND11_OVERLOAD(void, Operation, generateSummary, res); }

  // We incorporate the base class's method with a different access modifier
  using Operation::createBaseSpecification;

  /// Avoid PyGILState_Check() failures by running on the main thread by default.
  bool threadSafe() const override { return false; }

private:
  Specification createSpecification() override
  {
    Specification result;
    PyOperation::runOnMainThread([&]()
      {
        result = this->createSpecificationMainThread();
      }
    );
    return result;
  }
  Specification createSpecificationMainThread()
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

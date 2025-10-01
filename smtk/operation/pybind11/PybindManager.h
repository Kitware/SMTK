//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_Manager_h
#define pybind_smtk_operation_Manager_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/operation/Manager.h"

#include "smtk/operation/operators/ImportPythonOperation.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Operation.h"

#include "smtk/operation/pybind11/PyOperation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"

#include "smtk/io/Logger.h"

#include <chrono>
#include <vector>

namespace py = pybind11;

namespace
{

// Wait until \a cxx_future (an operation's result) is available, then set
// the \a py_future to the corresponding operation's outcome. Because the
// \a py_future is a special python object that yields rather than blocking
// until its result is available, the main thread is available for other work.
void future_awaiter_function(
  py::object eventLoop,
  py::object py_future,
  const std::shared_future<smtk::attribute::Attribute::Ptr>& cxx_future)
{
  auto result = cxx_future.get();
  auto outcome = smtk::operation::outcome(result);

  {
    // Acquire the GIL now and not earlier.
    pybind11::gil_scoped_acquire gil;
    eventLoop.attr("call_soon_threadsafe")(py_future.attr("set_result"), outcome);

    // In addition calling set_result on the future, we must hold the GIL when
    // decrementing the reference-count on the future and event-loop objects.
    // If we allow them to live past the end of this block, the reference-count
    // is decremented after we've released the GIL. So, manually decrement their
    // reference-counts and call release() so their destructors do not attempt
    // to re-decrement.

    // clang-format off
    py_future.dec_ref(); py_future.release();
    eventLoop.dec_ref(); eventLoop.release();
    // clang-format on
  }
}

} // anonymous namespace

inline PySharedPtrClass< smtk::operation::Manager > pybind11_init_smtk_operation_Manager(py::module& opModule)
{
  // First, wrap the future object returned by the manager's operation launcher.
  // We need this so that applications/tests can wait on operations to complete before exiting.
  py::class_<std::shared_future<smtk::attribute::Attribute::Ptr>> futureObject(opModule, "ResultFuture");
  futureObject
    .def("__await__", [](const std::shared_future<smtk::attribute::Attribute::Ptr>& self)
      {
        py::object loop = py::module::import("asyncio.events").attr("get_event_loop")();
        py::object py_future = loop.attr("create_future")();
        auto result = py_future.attr("__await__")();
        // Spawn a thread to resolve the future when the operation completes.
        std::thread py_future_setter(future_awaiter_function, loop, py_future, self);
        py_future_setter.detach();
        return result;
      }, R"(
      Return a python future's iterator so coroutines can yield until an operation is complete.
      This chains a C++ future with a python future to obtain the expected behavior.)"
    )
    .def("valid", [](const std::shared_future<smtk::attribute::Attribute::Ptr>& self) -> bool
      {
        return self.valid();
      }, R"(
      Return true if the operation-result's future is valid (i.e., has state tied to an operation).)"
    )
    .def("status", [](const std::shared_future<smtk::attribute::Attribute::Ptr>& self)
      {
        using namespace std::chrono_literals;
        switch (self.wait_for(0ms))
        {
        case std::future_status::ready: return "ready";
        case std::future_status::timeout: return "waiting";
        case std::future_status::deferred: return "deferred";
        }
        return "error";
      }, R"(
      Return a string indicating the state of the future. A string of "deferred" means
      that the future's value will not be computed until wait() is invoked.)"
    )
    .def("wait", &std::shared_future<smtk::attribute::Attribute::Ptr>::wait, R"(
      Block until the operation's result is ready to be processed.)"
    )
    .def("get", &std::shared_future<smtk::attribute::Attribute::Ptr>::get, R"(
      Return the operation result. This will block as needed.)"
    )
    ;

  // Wrap a C++ promise so it can be passed through python code.
  // This is needed so that smtk.operation.invokeObservers() can be passed a promise
  // that blocks the operation's thread until observers have completed running
  // (preventing locks from being released during observation).
  py::class_<std::promise<int>, std::shared_ptr<std::promise<int>>> intPromiseObject(opModule, "IntPromise");
  intPromiseObject
    .def("set_value",
      [](std::promise<int>& self, int value) { self.set_value(value); }, py::arg("value"), R"(
        Resolve the promise by providing the promised value.)"
    )
    .def("get_future", &std::promise<int>::get_future, R"(
      Return a C++ future object that blocks until set_value() is called.)"
    )
    ;

  // Now we can wrap the Manager class whose methods may produce "ResultFuture" and "IntPromise" objects.
  PySharedPtrClass< smtk::operation::Manager > instance(opModule, "Manager");
  instance
    .def("availableOperations", (std::set<std::string> (smtk::operation::Manager::*)() const) &smtk::operation::Manager::availableOperations)
    .def("availableOperations", (std::set<smtk::operation::Operation::Index> (smtk::operation::Manager::*)(const smtk::resource::ComponentPtr&) const) &smtk::operation::Manager::availableOperations)
    .def_static("create", (std::shared_ptr<smtk::operation::Manager> (*)()) &smtk::operation::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::operation::Manager> (*)(::std::shared_ptr<smtk::operation::Manager> &)) &smtk::operation::Manager::create, py::arg("ref"))
    .def("createOperation", (std::shared_ptr<smtk::operation::Operation> (smtk::operation::Manager::*)(::std::string const &)) &smtk::operation::Manager::create, py::arg("arg0"))
    .def("createOperation", (std::shared_ptr<smtk::operation::Operation> (smtk::operation::Manager::*)(::smtk::operation::Operation::Index const &)) &smtk::operation::Manager::create, py::arg("arg0"))
//    .def("metadata", [](smtk::operation::Manager& man) { std::vector<std::reference_wrapper<smtk::operation::Metadata>> vec; vec.reserve(man.metadata().size()); for (auto md : man.metadata()) { vec.push_back(md); } return vec; })
    .def("metadataObservers", (smtk::operation::Metadata::Observers & (smtk::operation::Manager::*)()) &smtk::operation::Manager::metadataObservers)
    .def("metadataObservers", (smtk::operation::Metadata::Observers const & (smtk::operation::Manager::*)() const) &smtk::operation::Manager::metadataObservers)
    .def("observers", (smtk::operation::Observers & (smtk::operation::Manager::*)()) &smtk::operation::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("observers", (smtk::operation::Observers const & (smtk::operation::Manager::*)() const) &smtk::operation::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("registered", (bool (smtk::operation::Manager::*)(const std::string&) const) &smtk::operation::Manager::registered, py::arg("typeName"))
    .def("registerResourceManager", &smtk::operation::Manager::registerResourceManager, py::arg("arg0"))
    .def("registerOperation", [](smtk::operation::Manager& manager, const std::string& moduleName, const std::string& opName){
        return smtk::operation::ImportPythonOperation::importOperation(manager, moduleName, opName);
      })
    .def("unregisterOperation", (bool (smtk::operation::Manager::*)(const std::string&)) &smtk::operation::Manager::unregisterOperation, py::arg("typeName"))
    .def("importOperationsFromModule", [](smtk::operation::Manager& manager, const std::string& moduleName)
      {
         return smtk::operation::ImportPythonOperation::importOperationsFromModule(moduleName, manager);
      }, py::arg("module"))
    .def("managers", &smtk::operation::Manager::managers)
    .def("setManagers", &smtk::operation::Manager::setManagers, py::arg("managers"))
    .def("launch", [](smtk::operation::Manager& manager, const std::shared_ptr<smtk::operation::Operation>& op)
      {
        // return manager.launchers()(op);
        auto resultFuture = manager.launchers()(op);
        // See if asyncio.events.get_event_loop() returns a loop that is running. If so,
        // we can create a future for the loop and queue a std::thread to set the future
        // upon completion of the operation. If not, ???. We still might be able to run
        // the operation in a separate thread, but (a) what should we return? and (b) how
        // can we ensure callbacks work? We might be able to detected whether a callback
        // override is set (to move execution from the operation thread to the interpreter
        // thread), but how can we use it to run python code when the std::shared_future<>
        // in \a resultFuture resolves? Say we are running with Qt.
        //
        // Now in a separate thread, block until the future is valid, then set the result.
        return resultFuture;
      }, py::arg("operation"), R"(
      Queue an operation to run in separate threads and return a future for its result.)")
    .def("overrideObserversUsingAsyncIO", [opModule](smtk::operation::Manager& manager, py::object eventLoop)
      {
        // If the \a eventLoop is None, fetch the running one on the current thread.
        auto asyncio = py::module::import("asyncio");
        if (eventLoop.is_none())
        {
          eventLoop = asyncio.attr("get_running_loop")();
        }
        // We must have an event loop to queue the observer-invocation on:
        if (eventLoop.is_none())
        {
          smtkErrorMacro(smtk::io::Logger::instance(), "Invalid event loop!");
          return false;
        }

        // Fetch the "promiseToRun" function. This must be a python callable
        // since it will be passed to "call_soon_threadsafe".
        auto promiseToRun = opModule.attr("promiseToRun");
        smtk::operation::PyOperation::runOnMainThread = [eventLoop, promiseToRun](std::function<void(void)> fn)
        {
          auto functionRun = std::make_shared<std::promise<int>>();
          auto done = functionRun->get_future();
          {
            // Acquire the GIL as call_soon_threadsafe needs it.
            pybind11::gil_scoped_acquire gil;
            eventLoop.attr("call_soon_threadsafe")(promiseToRun, fn, functionRun);
          }
          // Now wait until the function has been invoked on the eventLoop (i.e., the
          // local thread should block until the main thread has run the function).
          done.wait();
        };

        // Fetch the "invokeObservers" function. This must be a python callable
        // since it will be passed to "call_soon_threadsafe" (which is how one
        // adds tasks to an event loop).
        auto invokeObservers = opModule.attr("invokeObservers");
        auto self = manager.shared_from_this();
        // Provide the \a manager's observers with an override that blocks on
        // the calling thread until the observers have been invoked on the event-loop's
        // thread.
        manager.observers().overrideWith(
          [self, eventLoop, invokeObservers](
            const smtk::operation::Operation& op,
            smtk::operation::EventType event,
            smtk::operation::Operation::Result result) -> int
          {
            // Create a promise that invokeObservers resolves for us once all
            // the observers have been called. This prevents us from releasing
            // locks held on this thread by the operation.
            auto observersCalled = std::make_shared<std::promise<int>>();
            auto done = observersCalled->get_future();
            {
              // Acquire the GIL as call_soon_threadsafe needs it.
              pybind11::gil_scoped_acquire gil;
              eventLoop.attr("call_soon_threadsafe")(invokeObservers, op, event, result, self, observersCalled);
            }
            // Now wait until observers have been invoked on the eventLoop (so that
            // resource locks are held on this thread until the observers have all
            // completed).
            return done.get();
            // For debugging:
            //   auto dval = done.get();
            //   std::cerr << "Observer value " << dval << "\n";
            //   return dval;
          }
        );
        return true;
      }, py::arg("event_loop"), R"(
      Force operation observers to be called on the provided (or
      currently-running, if not provided) asyncio event loop.

      This method must be invoked on the thread owning the event loop.
      Then, no matter which thread an operation runs on, the observers
      will be invoked on the event loop of that thread.

      This allows python observers to be run on a thread where the
      interpreter has been initialized.
      )"
    )
    .def("removeObserversOverride", [](smtk::operation::Manager& manager)
      {
        manager.observers().removeOverride();
        smtk::operation::PyOperation::runOnMainThread = [](std::function<void(void)> fn)
        {
          fn();
        };
      }, R"(
      Remove any overrides for operation observers (including but not limited
      to the one created by calling manager.overrideObserversUsingAsyncIO()).)"
    )
    ;

  // Add an invokeObservers function to the module that calls observers directly
  // and resolves a promise so that resource locks on the operation's thread can
  // be released.
  opModule
    .def("invokeObservers",
      [](const smtk::operation::Operation::Ptr& op,
        smtk::operation::EventType event,
        const smtk::operation::Operation::Result& result,
        const smtk::operation::Manager::Ptr& operationManager,
        std::shared_ptr<std::promise<int>>& observersCalled)
      {
        // Release the GIL as any python observers will re-acquire it:
        pybind11::gil_scoped_release gil(true);
        // Invoke the observers (presumably this lambda is called from Python's event-loop thread).
        int status = -1;
        try {
          status = operationManager->observers().callObserversDirectly(*op, event, result);
        } catch (std::exception& e)
        {
          op->log().setFlushToStderr(true);
          smtkErrorMacro(
            op->log(),
            "An unhandled exception (" << e.what() << ") occurred in " << op->typeName() <<
            "processing observations via python asyncio.");
        }
        // For debugging:
        // std::cerr << "  observer status " << status << "\n";

        // Resolve the C++ promise which in turn resolves the Python future which then allows
        // any python coroutines/tasks which were awaiting the operation to run.
        observersCalled->set_value(status);
        return status;
      }
    )
    .def("promiseToRun",
      [](
        std::function<void(void)> fn,
        std::shared_ptr<std::promise<int>>& functionHasRun)
      {
        int status = -1;;
        {
          // Release the GIL; if \a fn is a python callable, it will re-acquire it:
          pybind11::gil_scoped_release gil(true);
          try {
            fn();
            status = 1;
          }
          catch (std::exception& e)
          {
            auto log = smtk::io::Logger::instance();
            log.setFlushToStderr(true);
            smtkErrorMacro(
              log, "An unhandled exception (" << e.what() << ") occurred running "
              "a user function on the main thread.");
            status = 0;
          }
        }
        functionHasRun->set_value(status);
      }, py::arg("callable"), py::arg("promise"), R"(
      Run the passed callable object. Once it completes, set the promise's
      value to 1 on success and 0 on failure. This method is for internal use
      only by smtk::operation::PyOperation::runOnMainThread() which may be
      called from C++ code run on any thread.)"
    )
  ;

  return instance;
}

#endif

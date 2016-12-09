//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_OperatorLog_h
#define pybind_smtk_io_OperatorLog_h

#include <pybind11/pybind11.h>

#include "smtk/io/OperatorLog.h"

namespace py = pybind11;

namespace smtk {
  namespace io {
class OperatorLog_ : public OperatorLog
{
public:
  OperatorLog_(smtk::model::ManagerPtr mgr) : OperatorLog(mgr) {}
  virtual ~OperatorLog_() {}

  virtual int recordInvocation(
    smtk::model::OperatorEventType event,
    const smtk::model::Operator& op) = 0;

  virtual int recordResult(
    smtk::model::OperatorEventType event,
    const smtk::model::Operator& op,
    smtk::model::OperatorResult r) = 0;
};
}
}

class PyOperatorLog : public smtk::io::OperatorLog_
{
public:
  using smtk::io::OperatorLog_::OperatorLog_;

  int recordInvocation(
    smtk::model::OperatorEventType event,
    const smtk::model::Operator& op) override
  {
    PYBIND11_OVERLOAD_PURE(int, smtk::io::OperatorLog_, recordInvocation, event, op);
  }

  int recordResult(
    smtk::model::OperatorEventType event,
    const smtk::model::Operator& op,
    smtk::model::OperatorResult r) override
  {
    PYBIND11_OVERLOAD_PURE(int, smtk::io::OperatorLog_, recordResult, event, op, r);
  }

};

PySharedPtrClass< smtk::io::OperatorLog > pybind11_init_smtk_io_OperatorLog(py::module &m)
{
  PySharedPtrClass< smtk::io::OperatorLog_, PyOperatorLog > instance(m, "OperatorLog");
  instance
    .def(py::init<smtk::model::ManagerPtr>())
    .def("deepcopy", (smtk::io::OperatorLog_ & (smtk::io::OperatorLog_::*)(::smtk::io::OperatorLog_ const &)) &smtk::io::OperatorLog_::operator=)
    .def("hasFailures", &smtk::io::OperatorLog_::hasFailures)
    .def("resetFailures", &smtk::io::OperatorLog_::resetFailures)
    .def("recordInvocation", &smtk::io::OperatorLog_::recordInvocation)
    .def("recordResult", &smtk::io::OperatorLog_::recordResult)
    ;
  return instance;
}

#endif

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

#include <pybind11/pybind11.h>

#include <type_traits>

#include "smtk/model/Operator.h"

#include "smtk/common/PythonInterpreter.h"

namespace smtk
{
namespace model
{
class PyOperator : public Operator
{
public:
  PyOperator() : Operator() {}
  virtual ~PyOperator() {}

  static std::shared_ptr<smtk::model::Operator> create(std::string modulename, std::string classname)
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

        std::shared_ptr<smtk::model::PyOperator> op(obj.cast<smtk::model::PyOperator*>());
        // Have the instance hold onto its python object, so when the instance
        // is removed so is the tether to the python object.
        op->setObject(obj);

        return std::static_pointer_cast<smtk::model::Operator>(op);
      }
      else
      {
        // Have the instance hold onto its python object, so when the instance
        // is removed so is the tether to the python object.
        obj.cast<std::shared_ptr<smtk::model::PyOperator> >()->setObject(obj);

        // If we are running in a native python instance (i.e. not our embedded
        // instance), then memory management is handled by python. We need only
        // to cast our python object into a shared_ptr so the SMTK operator
        // handling system can use it.
        return obj.cast<std::shared_ptr<smtk::model::Operator> >();
      }
    }

  std::string name() const override { PYBIND11_OVERLOAD_PURE(std::string, Operator, name, ); }
  std::string className() const override { PYBIND11_OVERLOAD_PURE(std::string, Operator, className, ); }
  bool ableToOperate() override { PYBIND11_OVERLOAD(bool, Operator, ableToOperate, ); }
  OperatorResult operate() override { PYBIND11_OVERLOAD(OperatorResult, Operator, operate, ); }

  OperatorResult operateInternal() override { PYBIND11_OVERLOAD_PURE(OperatorResult, Operator, operateInternal, ); }
  void generateSummary(OperatorResult& res) override { PYBIND11_OVERLOAD(void, Operator, generateSummary, res); }

  private:
  void setObject(pybind11::object obj) { m_object = obj; }

  pybind11::object m_object;
};
}
}

#endif

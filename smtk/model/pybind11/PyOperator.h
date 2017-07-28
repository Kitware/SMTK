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

      // Have the instance hold onto its python object. This way, we let the C++ side have control of memory allocation.
      obj.cast<std::shared_ptr<smtk::model::PyOperator> >()->setObject(obj);

      return obj.cast<std::shared_ptr<smtk::model::Operator> >();
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

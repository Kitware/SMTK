//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "WriteOperator.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Model.h"
#include "smtk/model/Manager.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItem.h"
#include "vtkModel.h"
#include <vtksys/SystemTools.hxx>

#include "WriteOperator_xml.h"

// #define SMTK_DISCRETE_SESSION_DEBUG

#if defined(SMTK_DISCRETE_SESSION_DEBUG)
#include "smtk/io/ExportJSON.h"
#include "cJSON.h"
#endif

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

WriteOperator::WriteOperator()
{
  this->m_currentversion = 5;
}

bool WriteOperator::ableToOperate()
{
  smtk::model::Model model;
  bool able2Op =
    this->ensureSpecification() &&
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")
      ->value().as<smtk::model::Model>()).isValid() &&
    // The CMB model must exist
    this->discreteSession()->findModelEntity(model.entity());
  if(!able2Op)
    return false;

  std::string filename = this->specification()->findFile("filename")->value();
  return !filename.empty();
}

OperatorResult WriteOperator::operateInternal()
{
  std::string fname = this->specification()->findFile("filename")->value();
  if (fname.empty())
    return this->createResult(OPERATION_FAILED);

  this->m_op->SetFileName(fname.c_str());
  Session* opsession = this->discreteSession();

  // ableToOperate should have verified that the model exists
  smtk::model::Model model = this->specification()->
    findModelEntity("model")->value().as<smtk::model::Model>();
  vtkDiscreteModelWrapper* modelWrapper =
    opsession->findModelEntity(model.entity());

  // write the file out.
  this->m_op->SetVersion(this->m_currentversion);
  this->m_op->Operate(modelWrapper);

  if (!this->m_op->GetOperateSucceeded())
    {
    std::cerr << "Could not write file \"" << fname << "\".\n";
    return this->createResult(OPERATION_FAILED);
    }

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  // The model was not modified while writing cmb file.
  // this->addEntityToResult(result, model, MODIFIED);

  return result;
}

Session* WriteOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::discrete::WriteOperator,
  discrete_write,
  "write",
  WriteOperator_xml,
  smtk::bridge::discrete::Session);

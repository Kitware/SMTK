//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ReadOperator.h"

#include "smtk/bridge/cmb/Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Operator.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItem.h"
#include "vtkModelEntity.h"

#include "ReadOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

ReadOperator::ReadOperator()
{
}

bool ReadOperator::ableToOperate()
{
  return
    this->ensureSpecification()
    ;
}

OperatorResult ReadOperator::operateInternal()
{
  std::string fname = this->specification()->findFile("filename")->value();
  if (fname.empty())
    return this->createResult(OPERATION_FAILED);

  this->m_op->SetFileName(fname.c_str());

  // Create a new model to hold the result.
  vtkNew<vtkDiscreteModelWrapper> mod;
  this->m_op->Operate(mod.GetPointer());

  // Now assign a UUID to the model and associate its filename with
  // a URL property (if things went OK).
  if (!this->m_op->GetOperateSucceeded())
    {
    std::cerr << "Could not read file \"" << fname << "\".\n";
    return this->createResult(OPERATION_FAILED);
    }

  smtk::common::UUID modelId = this->discreteBridge()->trackModel(
    mod.GetPointer(), fname, this->manager());
  smtk::model::Cursor modelEntity(this->manager(), modelId);

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  result->findModelEntity("model")->setValue(modelEntity);
/*
//#include "smtk/io/ExportJSON.h"
//#include "cJSON.h"

  cJSON* json = cJSON_CreateObject();
  smtk::io::ExportJSON::fromModel(json, this->manager());
  std::cout << "Result " << cJSON_Print(json) << "\n";
  cJSON_Delete(json);
  */
/*
std::string json = smtk::io::ExportJSON::fromModel(this->manager());
    std::ofstream file("/Users/yuminyuan/Desktop/smooth_surface1.json");
    file << json;
    file.close();
*/
  return result;
}

Bridge* ReadOperator::discreteBridge() const
{
  return dynamic_cast<Bridge*>(this->bridge());
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::discrete::ReadOperator,
  cmb_read,
  "read",
  ReadOperator_xml,
  smtk::bridge::discrete::Bridge);

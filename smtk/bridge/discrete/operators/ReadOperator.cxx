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

#include "ReadOperator_xml.h"

// #define SMTK_DISCRETE_SESSION_DEBUG

#if defined(SMTK_DISCRETE_SESSION_DEBUG)
#include "smtk/io/ExportJSON.h"
#include "cJSON.h"
#endif

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

ReadOperator::ReadOperator()
{
}

bool ReadOperator::ableToOperate()
{
  if(!this->specification()->isValid())
    return false;

  std::string filename = this->specification()->findFile("filename")->value();
  if (filename.empty())
    return false;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return ext == ".cmb";
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

  smtk::common::UUID modelId = this->discreteSession()->trackModel(
    mod.GetPointer(), fname, this->manager());
  smtk::model::EntityRef modelEntity(this->manager(), modelId);

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItemPtr models =
    result->findModelEntity("created");
  models->setNumberOfValues(1);
  models->setValue(0, modelEntity);

#if defined(SMTK_DISCRETE_SESSION_DEBUG)
  std::string json = smtk::io::ExportJSON::fromModelManager(this->manager());
  std::ofstream file("/tmp/read_op_out.json");
  file << json;
  file.close();
#endif

  modelEntity.as<smtk::model::Model>().setSession(
    smtk::model::SessionRef(
      this->manager(), this->session()->sessionId()));

  return result;
}

Session* ReadOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKDISCRETESESSION_EXPORT,
  smtk::bridge::discrete::ReadOperator,
  discrete_read,
  "read",
  ReadOperator_xml,
  smtk::bridge::discrete::Session);

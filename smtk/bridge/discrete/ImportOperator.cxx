//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ImportOperator.h"

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
#include "vtkPDataSetReader.h"
#include "vtkDataSetRegionSurfaceFilter.h"
#include "vtkMasterPolyDataNormals.h"
#include "vtkMergeDuplicateCells.h"

#include <vtksys/SystemTools.hxx>

#ifdef SMTK_BUILD_MOAB_READER
#include "smtk/bridge/discrete/moabreader/vtkCmbMoabReader.h"
#endif

#include "ModelParserHelper.h"
#include "ImportOperator_xml.h"

#include "smtk/io/ExportJSON.h"
#include "cJSON.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

ImportOperator::ImportOperator()
{
}

bool ImportOperator::ableToOperate()
{
  if(!this->specification()->isValid())
    return false;

  std::string filename = this->specification()->findFile("filename")->value();
  if (filename.empty())
    return false;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  bool able = (ext == ".vtk");
#ifdef SMTK_BUILD_MOAB_READER
    able = able || ext == ".exo";
#endif
  return able;
}

OperatorResult ImportOperator::operateInternal()
{
/*
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem =
    this->specification()->findString("filetype");

  std::string filename = filenameItem ?
    filenameItem->value() : "";
//  std::string filetype = filetypeItem ?
//    filetypeItem->value() : "";
*/
  std::string filename = this->specification()->findFile("filename")->value();

/*
  cJSON* json = cJSON_CreateObject();
  smtk::io::ExportJSON::forOperator(this->specification(), json);
  std::cout << "Import Operator: " << cJSON_Print(json) << "\n";
  cJSON_Delete(json);

*/
  if (filename.empty())
    {
    std::cerr << "File name is empty!\n";
    return this->createResult(OPERATION_FAILED);
    }

  // Create a new model to hold the result.
  vtkNew<vtkDiscreteModelWrapper> mod;

// ******************************************************************************
// This is where we should have the logic to import files other than .cmb formats
// ******************************************************************************
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  if(ext == ".exo")
    {
#ifdef SMTK_BUILD_MOAB_READER

    vtkNew<vtkCmbMoabReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    this->m_op->Operate(mod.GetPointer(), reader.GetPointer());
#endif
    }
  else if(ext == ".vtk")
    {
    vtkNew<vtkPDataSetReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkNew<vtkDataSetRegionSurfaceFilter> surface;
    surface->SetRegionArrayName(ModelParserHelper::GetShellTagName());
    surface->SetInputData(0, reader->GetOutputDataObject(0));
    surface->Update();

    vtkNew<vtkMasterPolyDataNormals> normals;
    normals->SetInputData(0, surface->GetOutputDataObject(0));
    normals->Update();

    vtkNew<vtkMergeDuplicateCells> merge;
    merge->SetModelRegionArrayName(ModelParserHelper::GetShellTagName());
    merge->SetModelFaceArrayName(ModelParserHelper::GetModelFaceTagName());
    merge->SetInputData(0, normals->GetOutputDataObject(0));
    merge->Update();

    this->m_op->Operate(mod.GetPointer(), merge.GetPointer());
    }

  // Now assign a UUID to the model and associate its filename with
  // a URL property (if things went OK).
  if (!this->m_op->GetOperateSucceeded())
    {
    std::cerr << "Failed to import file \"" << filename << "\".\n";
    return this->createResult(OPERATION_FAILED);
    }

  smtk::common::UUID modelId = this->discreteSession()->trackModel(
    mod.GetPointer(), filename, this->manager());
  smtk::model::EntityRef modelEntity(this->manager(), modelId);

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);

  smtk::attribute::ModelEntityItemPtr models =
    result->findModelEntity("entities");
  models->setNumberOfValues(1);
  models->setValue(0, modelEntity);

/*
//#include "smtk/io/ExportJSON.h"
//#include "cJSON.h"

  cJSON* json = cJSON_CreateObject();
  smtk::io::ExportJSON::fromModelManager(json, this->manager());
  std::cout << "Result " << cJSON_Print(json) << "\n";
  cJSON_Delete(json);
  */
/*
std::string json = smtk::io::ExportJSON::fromModelManager(this->manager());
    std::ofstream file("/tmp/import_op_out.json");
    file << json;
    file.close();
*/

  this->manager()->setSessionForModel(
    this->session()->shared_from_this(),
    modelId);

  return result;
}

Session* ImportOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::discrete::ImportOperator,
  discrete_import,
  "import",
  ImportOperator_xml,
  smtk::bridge::discrete::Session);

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

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelItem.h"

#include <vtksys/SystemTools.hxx>

#include "ReadOperator_xml.h"

// #define SMTK_DISCRETE_SESSION_DEBUG

#if defined(SMTK_DISCRETE_SESSION_DEBUG)
#include "cJSON.h"
#include "smtk/io/SaveJSON.h"
#endif

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

ReadOperator::ReadOperator()
{
}

bool ReadOperator::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
    return false;

  std::string filename = this->parameters()->findFile("filename")->value();
  if (filename.empty())
    return false;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return ext == ".cmb";
}

ReadOperator::Result ReadOperator::operateInternal()
{
  std::string fname = this->parameters()->findFile("filename")->value();
  if (fname.empty())
    return this->createResult(smtk::operation::NewOp::Outcome::FAILED);

  this->m_op->SetFileName(fname.c_str());
  std::string modelName = vtksys::SystemTools::GetFilenameWithoutExtension(fname.c_str());

  // There are three possible import modes
  //
  // 1. Import a mesh into an existing resource
  // 2. Import a mesh as a new model, but using the session of an existing resource
  // 3. Import a mesh into a new resource

  smtk::bridge::discrete::Resource::Ptr resource = nullptr;
  smtk::bridge::discrete::Session::Ptr session = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ResourceItem::Ptr existingResourceItem =
    this->parameters()->findResource("resource");

  if (existingResourceItem && existingResourceItem->isEnabled())
  {
    smtk::bridge::discrete::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::bridge::discrete::Resource>(existingResourceItem->value());

    session = existingResource->discreteSession();

    smtk::attribute::StringItem::Ptr sessionOnlyItem =
      this->parameters()->findString("session only");
    if (sessionOnlyItem->value() == "this file")
    {
      // If the "session only" value is set to "this file", then we use the
      // existing resource
      resource = existingResource;
    }
    else
    {
      // If the "session only" value is set to "this session", then we create a
      // new resource with the session from the exisiting resource
      resource = smtk::bridge::discrete::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::bridge::discrete::Resource::create();
    session = smtk::bridge::discrete::Session::create();

    // Create a new resource for the import
    resource->setLocation(fname);
    resource->setSession(session);
  }

  // Create a new model to hold the result.
  vtkNew<vtkDiscreteModelWrapper> mod;
  this->m_op->Operate(mod.GetPointer(), session.get());

  // Now assign a UUID to the model and associate its filename with
  // a URL property (if things went OK).
  if (!this->m_op->GetOperateSucceeded())
  {
    std::cerr << "Could not read file \"" << fname << "\".\n";
    return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
  }

  smtk::common::UUID modelId = session->trackModel(mod.GetPointer(), fname, resource);
  smtk::model::EntityRef modelEntity(resource, modelId);
  modelEntity.setName(modelName);

  OperatorResult result = this->createResult(smtk::operation::NewOp::Outcome::SUCCEEDED);

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->appendValue(modelEntity.component());
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
    resultModels->setValue(modelEntity.component());
  }

#if defined(SMTK_DISCRETE_SESSION_DEBUG)
  std::string json = smtk::io::SaveJSON::fromModelManager(resource);
  std::ofstream file("/tmp/read_op_out.json");
  file << json;
  file.close();
#endif

  modelEntity.as<smtk::model::Model>().setSession(
    smtk::model::SessionRef(resource, session->sessionId()));

  return result;
}

const char* ReadOperator::xmlDescription() const
{
  return ReadOperator_xml;
}

} // namespace discrete
} // namespace bridge

} // namespace smtk

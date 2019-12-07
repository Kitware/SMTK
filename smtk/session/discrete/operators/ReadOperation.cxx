//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ReadOperation.h"

#include "smtk/session/discrete/Resource.h"
#include "smtk/session/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/session/discrete/kernel/Model/vtkModel.h"
#include "smtk/session/discrete/kernel/Model/vtkModelItem.h"
#include "vtkDiscreteModelWrapper.h"

#include <vtksys/SystemTools.hxx>

#include "ReadOperation_xml.h"

// #define SMTK_DISCRETE_SESSION_DEBUG

#if defined(SMTK_DISCRETE_SESSION_DEBUG)
#include "smtk/io/SaveJSON.h"
#endif

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace discrete
{

ReadOperation::ReadOperation() = default;

bool ReadOperation::ableToOperate()
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

ReadOperation::Result ReadOperation::operateInternal()
{
  std::string fname = this->parameters()->findFile("filename")->value();
  if (fname.empty())
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  m_op->SetFileName(fname.c_str());
  std::string modelName = vtksys::SystemTools::GetFilenameWithoutExtension(fname);

  // There are three possible import modes
  //
  // 1. Import a mesh into an existing resource
  // 2. Import a mesh as a new model, but using the session of an existing resource
  // 3. Import a mesh into a new resource

  smtk::session::discrete::Resource::Ptr resource = nullptr;
  smtk::session::discrete::Session::Ptr session = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ResourceItem::Ptr existingResourceItem =
    this->parameters()->findResource("resource");

  if (existingResourceItem && existingResourceItem->isEnabled())
  {
    smtk::session::discrete::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::session::discrete::Resource>(existingResourceItem->value());

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
      resource = smtk::session::discrete::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::session::discrete::Resource::create();
    session = smtk::session::discrete::Session::create();

    // Create a new resource for the import
    resource->setLocation(fname);
    resource->setSession(session);
  }

  // Create a new model to hold the result.
  vtkNew<vtkDiscreteModelWrapper> mod;
  m_op->Operate(mod.GetPointer(), session.get());

  // Now assign a UUID to the model and associate its filename with
  // a URL property (if things went OK).
  if (!m_op->GetOperateSucceeded())
  {
    std::cerr << "Could not read file \"" << fname << "\".\n";
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::common::UUID modelId = session->trackModel(mod.GetPointer(), fname, resource);
  smtk::model::EntityRef modelEntity(resource, modelId);
  modelEntity.setName(modelName);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

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
  std::string json = smtk::io::SaveJSON::fromModelResource(resource);
  std::ofstream file("/tmp/read_op_out.json");
  file << json;
  file.close();
#endif

  modelEntity.as<smtk::model::Model>().setSession(
    smtk::model::SessionRef(resource, session->sessionId()));

  return result;
}

const char* ReadOperation::xmlDescription() const
{
  return ReadOperation_xml;
}

} // namespace discrete
} // namespace session

} // namespace smtk

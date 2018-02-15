//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/mesh/operators/Import.h"

#include "smtk/bridge/mesh/Resource.h"
#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/Paths.h"

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/resource/Manager.h"

#include "smtk/bridge/mesh/Import_xml.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace bridge
{
namespace mesh
{

Import::Result Import::operateInternal()
{
  // Get the read file name
  smtk::attribute::FileItem::Ptr filePathItem = this->parameters()->findFile("filename");
  std::string filePath = filePathItem->value();

  // Get the label item
  smtk::attribute::StringItem::Ptr labelItem = this->parameters()->findString("label");
  std::string label = labelItem->value();

  // Check whether or not a model hierarchy should be constructed
  smtk::attribute::VoidItem::Ptr hierarchyItem =
    this->parameters()->findVoid("construct hierarchy");
  bool constructHierarchy = hierarchyItem->isEnabled();

  // There are three possible import modes
  //
  // 1. Import a mesh into an existing resource
  // 2. Import a mesh as a new model, but using the session of an existing resource
  // 3. Import a mesh into a new resource

  smtk::bridge::mesh::Resource::Ptr resource = nullptr;
  smtk::bridge::mesh::Session::Ptr session = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ResourceItem::Ptr existingResourceItem =
    this->parameters()->findResource("resource");

  if (existingResourceItem && existingResourceItem->isEnabled())
  {
    smtk::bridge::mesh::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::bridge::mesh::Resource>(existingResourceItem->value());

    session = existingResource->session();

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
      resource = smtk::bridge::mesh::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::bridge::mesh::Resource::create();
    session = smtk::bridge::mesh::Session::create();

    // Create a new resource for the import
    resource->setLocation(filePath);
    resource->setSession(session);
  }

  // Get the collection from the file
  smtk::mesh::CollectionPtr collection = smtk::io::importMesh(filePath, resource->meshes(), label);

  // Name the mesh according to the stem of the file
  std::string name = smtk::common::Paths::stem(filePath);
  if (!name.empty())
  {
    collection->name(name);
  }

  if (!collection || !collection->isValid())
  {
    // The file was not correctly read.
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto format = smtk::io::meshFileFormat(filePath);
  if (format.Name == "exodus")
  {
    session->facade()["domain"] = "Element Block";
    session->facade()["dirichlet"] = "Node Set";
    session->facade()["neumann"] = "Side Set";
  }

  // Assign its model manager to the one associated with this session
  collection->setModelManager(resource);

  // Construct the topology
  session->addTopology(Topology(collection, constructHierarchy));

  // Determine the model's dimension
  int dimension = int(smtk::mesh::utility::highestDimension(collection->meshes()));

  // Our collections will already have a UUID, so here we create a model given
  // the model manager and uuid
  smtk::model::Model model = resource->insertModel(collection->entity(), dimension, dimension);

  // Name the model according to the stem of the file
  if (!name.empty())
  {
    model.setName(name);
  }

  // Set the url and type of the model
  model.setStringProperty("url", filePath);
  model.setStringProperty("type", format.Name);

  // Declare the model as "dangling" so it will be transcribed
  session->declareDanglingEntity(model);

  collection->associateToModel(model.entity());

  // Set the model's session to point to the current session
  model.setSession(smtk::model::SessionRef(resource, resource->session()->sessionId()));

  // If we don't call "transcribe" ourselves, it never gets called.
  resource->session()->transcribe(model, smtk::model::SESSION_EVERYTHING, false);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
  resultModels->setValue(model.component());

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->appendValue(model.component());
  }

  result->findComponent("mesh_created")->setValue(model.component());

  return result;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}

} // namespace mesh
} //namespace bridge
} // namespace smtk

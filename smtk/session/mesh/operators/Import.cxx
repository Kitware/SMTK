//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/Paths.h"

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/Manager.h"

#include "smtk/session/mesh/operators/Import_xml.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace session
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

  smtk::session::mesh::Resource::Ptr resource = nullptr;
  smtk::session::mesh::Session::Ptr session = nullptr;
  smtk::mesh::Resource::Ptr meshResource = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ReferenceItem::Ptr existingResourceItem = this->parameters()->associations();

  bool newResource = true;
  if (existingResourceItem->numberOfValues() > 0)
  {
    smtk::session::mesh::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::session::mesh::Resource>(existingResourceItem->value());

    session = existingResource->session();

    smtk::attribute::StringItem::Ptr sessionOnlyItem =
      this->parameters()->findString("session only");
    if (sessionOnlyItem->value() == "import into this file")
    {
      // If the "session only" value is set to "this file", then we use the
      // existing resource
      newResource = false;
      resource = existingResource;
      meshResource = existingResource->resource();

      // It is possible that a mesh session resource does not have a valid
      // mesh resource (for example, when it is newly created). In this case, we
      // need a new mesh resource instead.
      if (meshResource == nullptr)
      {
        newResource = true;
        meshResource = smtk::mesh::Resource::create();
      }
    }
    else
    {
      // If the "session only" value is set to "this session", then we create a
      // new resource with the session from the exisiting resource
      resource = smtk::session::mesh::Resource::create();
      meshResource = smtk::mesh::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::session::mesh::Resource::create();
    session = smtk::session::mesh::Session::create();
    meshResource = smtk::mesh::Resource::create();

    // Create a new resource for the import
    resource->setSession(session);
  }

  if (!meshResource || !meshResource->isValid())
  {
    // The file was not correctly read.
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Get the mesh resource from the file
  smtk::mesh::MeshSet preexistingMeshes = meshResource->meshes();
  smtk::io::importMesh(filePath, meshResource, label);
  smtk::mesh::MeshSet allMeshes = meshResource->meshes();
  smtk::mesh::MeshSet newMeshes = smtk::mesh::set_difference(allMeshes, preexistingMeshes);

  // Name the mesh according to the stem of the file
  std::string name = smtk::common::Paths::stem(filePath);
  if (!name.empty() && newResource)
  {
    meshResource->setName(name);
    resource->setName(name);
  }

  auto format = smtk::io::meshFileFormat(filePath);
  if (format.Name.find("Exodus") != std::string::npos)
  {
    session->facade()["domain"] = "Element Block";
    session->facade()["dirichlet"] = "Node Set";
    session->facade()["neumann"] = "Side Set";
  }

  // Assign the mesh resource's model resource to the one associated with this
  // session
  meshResource->setModelResource(resource);

  // Also assign the mesh resource to be the model's tessellation
  resource->setMeshTessellations(meshResource);

  // If we are reading a mesh session resource (as opposed to a new import), we
  // should access the existing model instead of creating a new one here. If
  // this is the case, then the mesh resource's associated model id will be related
  // to a model entity that is already in the resource (as it was put there by
  // the Read operation calling this one).
  smtk::common::UUID associatedModelId = meshResource->associatedModel();

  // By default, a model is invalid
  smtk::model::Model model;
  if (associatedModelId != smtk::common::UUID::null() && newResource)
  {
    // Assign the model to one described already in the resource with the id of
    // mesh resource's associated model. If there is no such model, then this
    // instance will also be invalid.
    model = smtk::model::Model(resource, associatedModelId);
  }

  if (!model.isValid())
  {
    // Determine the model's dimension
    int dimension = int(smtk::mesh::utility::highestDimension(meshResource->meshes()));

    // Create a model with the appropriate dimension
    model = resource->addModel(dimension, dimension);

    // Name the model according to the stem of the file
    if (!name.empty())
    {
      model.setName(name);
    }
  }

  // Construct the topology
  session->addTopology(resource, Topology(model.entity(), newMeshes, constructHierarchy));

  // Set the url and type of the model
  model.setStringProperty("url", filePath);
  model.setStringProperty("type", format.Name);

  // Declare the model as "dangling" so it will be transcribed
  session->declareDanglingEntity(model);

  meshResource->associateToModel(model.entity());

  // Set the model's session to point to the current session
  model.setSession(smtk::model::SessionRef(resource, resource->session()->sessionId()));

  // If we don't call "transcribe" ourselves, it never gets called.
  resource->session()->transcribe(
    model,
    (this->callFromRead ? smtk::model::SESSION_EVERYTHING & ~smtk::model::SESSION_PROPERTIES
                        : smtk::model::SESSION_EVERYTHING),
    false);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
  resultModels->setValue(model.component());

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->appendValue(model.component());
  }

  // Access the model resource's associated topology.
  smtk::session::mesh::Topology* topology = resource->session()->topology(resource);

  // Mark the modified model components to update their representative geometry
  smtk::operation::MarkGeometry markGeometry(resource);

  std::function<void(const smtk::common::UUID&)> mark;

  mark = [&](const smtk::common::UUID& id) {
    markGeometry.markModified(resource->find(id));
    auto elementIt = topology->m_elements.find(id);
    if (elementIt == topology->m_elements.end())
    {
      return;
    }
    Topology::Element& element = elementIt->second;

    for (const smtk::common::UUID& childId : element.m_children)
    {
      mark(childId);
    }
  };

  mark(model.entity());

  result->findComponent("mesh_created")->setValue(model.component());

  return result;
}

Import::Specification Import::createSpecification()
{
  Specification spec = this->smtk::operation::XMLOperation::createSpecification();
  auto importDef = spec->findDefinition("import");

  std::vector<smtk::attribute::FileItemDefinition::Ptr> fileItemDefinitions;
  auto fileItemDefinitionFilter = [](smtk::attribute::FileItemDefinition::Ptr ptr) {
    return ptr->name() == "filename";
  };
  importDef->filterItemDefinitions(fileItemDefinitions, fileItemDefinitionFilter);

  assert(fileItemDefinitions.size() == 1);

  std::stringstream fileFilters;
  bool firstFormat = true;
  for (auto& ioType : smtk::io::ImportMesh::SupportedIOTypes())
  {
    for (const auto& format : ioType->FileFormats())
    {
      if (format.CanImport())
      {
        if (firstFormat)
        {
          firstFormat = false;
        }
        else
        {
          fileFilters << ";;";
        }

        fileFilters << format.Name << "(";
        bool first = true;
        for (const auto& ext : format.Extensions)
        {
          if (first)
          {
            first = false;
          }
          else
          {
            fileFilters << " ";
          }
          fileFilters << "*" << ext;
        }
        fileFilters << ")";
      }
    }
  }
  fileItemDefinitions[0]->setFileFilters(fileFilters.str());
  return spec;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}

} // namespace mesh
} //namespace session
} // namespace smtk

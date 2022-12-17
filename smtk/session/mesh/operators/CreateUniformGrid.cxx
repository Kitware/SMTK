//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/mesh/operators/CreateUniformGrid.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Session.h"
#include "smtk/session/mesh/operators/CreateUniformGrid_xml.h"

#include "smtk/common/UUID.h"

#include "smtk/mesh/core/CellTraits.h"
#include "smtk/mesh/core/CellTypes.h"

#include "smtk/mesh/utility/Create.h"

namespace smtk
{
namespace session
{
namespace mesh
{

CreateUniformGrid::Result CreateUniformGrid::operateInternal()
{
  // Access the string describing the dimension. The dimension is a string so we
  // can use optional children to assign the lengths of the other parameters.
  smtk::attribute::StringItem::Ptr dimensionItem = this->parameters()->findString("dimension");

  int dimension;
  std::string suffix;
  if (dimensionItem->value() == "2")
  {
    dimension = 2;
    suffix = "2d";
  }
  else
  {
    dimension = 3;
    suffix = "3d";
  }

  // Access the origin, size and discretization parameters
  smtk::attribute::DoubleItemPtr originItem = this->parameters()->findDouble("origin" + suffix);
  smtk::attribute::DoubleItemPtr sizeItem = this->parameters()->findDouble("size" + suffix);
  smtk::attribute::IntItemPtr discretizationItem =
    this->parameters()->findInt("discretization" + suffix);

  // Copy their values to local fields
  std::array<double, 3> origin{ { 0., 0., 0. } };
  std::array<double, 3> size{ { 0., 0., 0. } };
  std::array<std::size_t, 3> discretization{ { 1, 1, 1 } };

  for (int i = 0; i < dimension; i++)
  {
    origin[i] = originItem->value(i);
    size[i] = sizeItem->value(i);
    discretization[i] = discretizationItem->value(i);
  }

  // Construct a mapping from a unit box to the input box
  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    return std::array<double, 3>(
      { { origin[0] + size[0] * x[0], origin[1] + size[1] * x[1], origin[2] + size[2] * x[2] } });
  };

  // There are three possible import modes
  //
  // 1. Import a mesh into an existing resource
  // 2. Import a mesh as a new model, but using the session of an existing resource
  // 3. Import a mesh into a new resource

  smtk::session::mesh::Resource::Ptr resource = nullptr;
  smtk::session::mesh::Session::Ptr session = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ResourceItem::Ptr existingResourceItem =
    this->parameters()->findResource("resource");

  if (existingResourceItem && existingResourceItem->isEnabled())
  {
    smtk::session::mesh::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::session::mesh::Resource>(existingResourceItem->value());

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
      resource = smtk::session::mesh::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::session::mesh::Resource::create();
    session = smtk::session::mesh::Session::create();

    // Create a new resource for the import
    resource->setSession(session);
  }

  // Create a new mesh mesh resource
  smtk::mesh::ResourcePtr meshResource = smtk::mesh::Resource::create();

  // Construct a uniform grid
  std::vector<smtk::mesh::MeshSet> meshes;
  if (dimension == 2)
  {
    std::array<std::size_t, 2> disc = { { discretization[0], discretization[1] } };
    auto ms = smtk::mesh::utility::createUniformGrid(meshResource, disc, fn);
    for (auto& m : ms)
    {
      meshes.push_back(m);
    }
  }
  else
  {
    auto ms = smtk::mesh::utility::createUniformGrid(meshResource, discretization, fn);
    for (auto& m : ms)
    {
      meshes.push_back(m);
    }
  }

  // Assign the mesh resource's model resource to the one associated with this
  // session.
  meshResource->setModelResource(resource);

  // Also assign the mesh resource to be the model's tessellation
  resource->setMeshTessellations(meshResource);

  // Create a model with the appropriate dimension
  smtk::model::Model model = resource->addModel(3, 3);

  // Construct the topology.
  session->addTopology(
    smtk::session::mesh::Topology(model.entity(), meshResource->meshes(), false));

  // Declare the model as "dangling" so it will be transcribed.
  session->declareDanglingEntity(model);

  meshResource->associateToModel(model.entity());

  // Set the model's session to point to the current session.
  model.setSession(smtk::model::SessionRef(resource, resource->session()->sessionId()));

  // If we don't call "transcribe" ourselves, it never gets called.
  resource->session()->transcribe(model, smtk::model::SESSION_EVERYTHING, false);

  // Now that there are model components associated with our mesh sets, give the
  // model adjacencies human-readable names.
  if (dimension == 2)
  {
    smtk::model::EntityRef(resource, meshes[0].modelEntityIds().at(0)).setName("Domain");
    smtk::model::EntityRef(resource, meshes[1].modelEntityIds().at(0)).setName("Lower X");
    smtk::model::EntityRef(resource, meshes[2].modelEntityIds().at(0)).setName("Lower Y");
    smtk::model::EntityRef(resource, meshes[3].modelEntityIds().at(0)).setName("Upper X");
    smtk::model::EntityRef(resource, meshes[4].modelEntityIds().at(0)).setName("Upper Y");
  }
  else
  {
    smtk::model::EntityRef(resource, meshes[0].modelEntityIds().at(0)).setName("Domain");
    smtk::model::EntityRef(resource, meshes[1].modelEntityIds().at(0)).setName("Lower X");
    smtk::model::EntityRef(resource, meshes[2].modelEntityIds().at(0)).setName("Lower Y");
    smtk::model::EntityRef(resource, meshes[3].modelEntityIds().at(0)).setName("Lower Z");
    smtk::model::EntityRef(resource, meshes[4].modelEntityIds().at(0)).setName("Upper X");
    smtk::model::EntityRef(resource, meshes[5].modelEntityIds().at(0)).setName("Upper Y");
    smtk::model::EntityRef(resource, meshes[6].modelEntityIds().at(0)).setName("Upper Z");
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->appendValue(model.component());
  }

  return result;
}

const char* CreateUniformGrid::xmlDescription() const
{
  return CreateUniformGrid_xml;
}
} // namespace mesh
} // namespace session
} // namespace smtk

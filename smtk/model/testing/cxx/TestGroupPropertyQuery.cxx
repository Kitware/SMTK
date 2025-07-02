//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/resource/Component.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <array>

namespace
{
// SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
} // namespace

// This test is designed to check that an empty group is not returned when a
// query is performed that filters for a model component type.
int TestGroupPropertyQuery(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Import a mesh as a mesh model
  smtk::session::mesh::Resource::Ptr resource;
  smtk::model::Model model;
  smtk::model::Face face;
  {
    smtk::operation::Operation::Ptr importOp = smtk::session::mesh::Import::create();

    if (!importOp)
    {
      std::cerr << "No import operator\n";
      return 1;
    }

    {
      std::string file_path(data_root);
      file_path += "/mesh/2d/Simple.2dm";
      importOp->parameters()->findFile("filename")->setValue(file_path);
      std::cout << "Importing " << file_path << "\n";
    }

    smtk::operation::Operation::Result importOpResult = importOp->operate();
    if (
      importOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Import operator failed\n";
      return 1;
    }

    // Retrieve the resulting resource
    smtk::attribute::ResourceItemPtr resourceItem =
      std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
        importOpResult->findResource("resourcesCreated"));

    // Access the generated resource
    resource = std::dynamic_pointer_cast<smtk::session::mesh::Resource>(resourceItem->value());

    // Retrieve the resulting model
    smtk::attribute::ComponentItemPtr componentItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
        importOpResult->findComponent("model"));

    // Access the generated model
    smtk::model::Entity::Ptr modelEnt =
      std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

    model = modelEnt->referenceAs<smtk::model::Model>();

    auto faceItem = importOpResult->findComponent("mesh_created");

    if (!faceItem || !faceItem->isValid())
    {
      std::cerr << "No associated mesh!\n";
      return 1;
    }

    face = faceItem->valueAs<smtk::model::Entity>();
  }

  // add an auxiliary geometry
  smtk::model::AuxiliaryGeometry auxGeo;
  {
    auto auxGeoOp = smtk::model::AddAuxiliaryGeometry::create();

    {
      std::string file_path(data_root);
      file_path += "/mesh/2d/SimpleBathy.2dm";
      auxGeoOp->parameters()->findFile("url")->setValue(file_path);
    }

    auxGeoOp->parameters()->associateEntity(model);
    smtk::operation::Operation::Result auxGeoOpResult = auxGeoOp->operate();
    if (
      auxGeoOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Add auxiliary geometry failed!\n";
      return 1;
    }

    // Retrieve the resulting auxiliary geometry item
    smtk::attribute::ComponentItemPtr auxGeoItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
        auxGeoOpResult->findComponent("created"));

    // Access the generated auxiliary geometry
    smtk::model::Entity::Ptr auxGeoEnt =
      std::dynamic_pointer_cast<smtk::model::Entity>(auxGeoItem->value());

    auxGeo = auxGeoEnt->referenceAs<smtk::model::AuxiliaryGeometry>();
    if (!auxGeo.isValid())
    {
      std::cerr << "Auxiliary geometry is not valid!\n";
      return 1;
    }
  }

  // Add a group
  smtk::model::Group group = resource->addGroup(0, "Bits'n'pieces");

  // Assign string properties to things
  face.setStringProperty("foo", "baz");
  auxGeo.setStringProperty("foo", "bar");
  group.setStringProperty("foo", "bar");

  // q1: should return 3 components (face, auxGeo and group all have string
  //     property "foo")
  auto q1 = smtk::model::Entity::filterStringToQueryFunctor("any [ string { 'foo' }]");

  // q2: should return 2 components (auxGeo and group both have string property
  //     "foo" with value "bar")
  auto q2 = smtk::model::Entity::filterStringToQueryFunctor("any [ string { 'foo' = 'bar' }]");

  // q3: should return 1 component (only auxGeo is an aux_geom with string
  //     property "foo")
  auto q3 = smtk::model::Entity::filterStringToQueryFunctor("aux_geom [ string { 'foo' }]");

  // q3: should return 1 component (only auxGeo is an aux_geom with string
  //     property "foo" and value "bar")
  auto q4 = smtk::model::Entity::filterStringToQueryFunctor("aux_geom [ string { 'foo' = 'bar' }]");

  // q3: should return 0 components (there are no aux_geom components with
  //     string property "foo" and value "baz")
  auto q5 = smtk::model::Entity::filterStringToQueryFunctor("aux_geom [ string { 'foo' = 'baz' }]");

  std::array<decltype(q1), 5> queries{ q1, q2, q3, q4, q5 };
  std::array<int, 5> expected{ 3, 2, 1, 1, 0 };
  std::array<int, 5> counts;
  std::fill(counts.begin(), counts.end(), 0);

  smtk::resource::Component::Visitor visitor = [&](const smtk::resource::ComponentPtr& component) {
    for (std::size_t i = 0; i < queries.size(); i++)
    {
      if (queries[i](*component))
      {
        ++counts[i];
      }
    }
  };
  resource->visit(visitor);

  for (auto& count : counts)
  {
    std::cout << count << std::endl;
  }

  smtkTest(
    std::equal(counts.begin(), counts.end(), expected.begin()),
    "query string returned unexpected result");

  return 0;
}

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
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"
#include "smtk/model/operators/CreateInstances.h"

#include "smtk/operation/Manager.h"

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
} // namespace

using namespace smtk::model;

// Import a triangulated terrain as a mesh model, add an auxiliary geometry as
// a glyph prototype, and create a bunch of glyph instances that are sampled on
// the surface of the terrain.
int TestSamplePointsOnSurface(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  auto importOp = smtk::session::mesh::Import::create();

  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  std::string importFilePath(data_root);
  importFilePath += "/mesh/3d/terrain.exo";

  importOp->parameters()->findFile("filename")->setValue(importFilePath);
  importOp->parameters()->findVoid("construct hierarchy")->setIsEnabled(false);

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
  smtk::session::mesh::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::session::mesh::Resource>(resourceItem->value());

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr modelEntity =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  smtk::model::Model model = modelEntity->referenceAs<smtk::model::Model>();

  if (!model.isValid())
  {
    std::cerr << "Import operator returned an invalid model\n";
    return 1;
  }

  // Access all of the model's faces
  smtk::model::EntityRefs currentEnts =
    resource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::FACE);
  if (currentEnts.empty())
  {
    std::cerr << "No faces!" << std::endl;
    return 1;
  }

  // Grab the top face (known to be named "Element Block 13")
  smtk::model::EntityRef eRef;
  for (const auto& e : currentEnts)
  {
    if (e.name() == "Element Block 13")
    {
      eRef = e;
    }
  }

  const smtk::model::Face& face = eRef.as<smtk::model::Face>();

  if (!face.isValid())
  {
    std::cerr << "Face is invald\n";
    return 1;
  }

  // add auxiliary geometry
  auto auxGeoOp = smtk::model::AddAuxiliaryGeometry::create();

  {
    std::string file_path(data_root);
    file_path += "/model/3d/obj/cone.obj";
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
  smtk::model::Entity::Ptr auxGeoEntity =
    std::dynamic_pointer_cast<smtk::model::Entity>(auxGeoItem->value());

  smtk::model::AuxiliaryGeometry auxGeo =
    auxGeoEntity->referenceAs<smtk::model::AuxiliaryGeometry>();

  if (!auxGeo.isValid())
  {
    std::cerr << "Auxiliary geometry is not valid!\n";
    return 1;
  }

  {
    // create the create instances operation
    auto createInstances = smtk::model::CreateInstances::create();
    if (!createInstances)
    {
      std::cerr << "No \"Create Instances\" operation!\n";
      return 1;
    }

    // set input values for the create instances operation
    createInstances->parameters()->associate(auxGeoEntity);
    createInstances->parameters()->findString("placement rule")->setDiscreteIndex(2);
    createInstances->parameters()->findComponent("surface")->setValue(face.component());
    createInstances->parameters()->findInt("sample size")->setValue(10);
    createInstances->parameters()->findString("snap to entity")->setIsEnabled(false);

    smtk::operation::Operation::Result result = createInstances->operate();
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Create Instances operation failed\n";
      return 1;
    }
  }

  // Iterate over the coordinates of the instances to ensure that their
  // coordinates snapped to the model surface, which is below the box where
  // random points were seeded.
  smtk::model::EntityRefs instances =
    resource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::INSTANCE_ENTITY);
  if (instances.empty())
  {
    std::cerr << "No instances!" << std::endl;
    return 1;
  }

  std::cout << "There are " << instances.size() << " instances" << std::endl;

  for (const auto& instance : instances)
  {
    const std::vector<double>& coords = instance.hasTessellation()->coords();
    for (std::size_t i = 0; i < coords.size(); i += 3)
    {
      {
        std::cout << "coordinate " << i / 3 << " = (" << coords[i] << ", " << coords[i + 1] << ", "
                  << coords[i + 2] << ")" << std::endl;
      }
    }
  }

  return 0;
}

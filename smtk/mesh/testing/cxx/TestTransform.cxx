//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/geometry/queries/BoundingBox.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/Import.h"
#include "smtk/mesh/operators/Transform.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace
{
// SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

// A numerical tolerance
const double epsilon = 1.e-8;
} // namespace

// Import a tetrahedralized cube, compute its extent, transform the cube and
// compare the new extent with the original one..
int TestTransform(int, char*[])
{
  // Import a tetrahedralized cube
  smtk::mesh::Resource::Ptr resource;
  {
    smtk::operation::Operation::Ptr importOp = smtk::mesh::Import::create();

    test(importOp != nullptr, "No import operator");

    {
      std::string file_path(data_root);
      file_path += "/mesh/3d/cube.exo";
      importOp->parameters()->findFile("filename")->setValue(file_path);
    }

    smtk::operation::Operation::Result importOpResult = importOp->operate();
    test(
      importOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Import operator failed");

    // Retrieve the resulting resource
    smtk::attribute::ResourceItemPtr resourceItem =
      std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
        importOpResult->findResource("resourcesCreated"));

    // Access the generated resource
    resource = std::dynamic_pointer_cast<smtk::mesh::Resource>(resourceItem->value());
  }

  auto& boundingBox = resource->queries().get<smtk::geometry::BoundingBox>();
  std::array<double, 6> extent = boundingBox(resource);

  {
    smtk::operation::Operation::Ptr transform = smtk::mesh::Transform::create();
    transform->parameters()->associate(smtk::mesh::Component::create(resource->meshes()));
    transform->parameters()->findDouble("scale")->setValue(1, 2.);

    smtk::operation::Operation::Result result = transform->operate();
    test(
      result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "transform failed");
  }

  std::array<double, 6> newExtent = smtk::mesh::utility::extent(resource->meshes());

  // Ensure that the extent doubled in the y direction
  test(fabs(extent[0] - newExtent[0]) < epsilon);
  test(fabs(extent[1] - newExtent[1]) < epsilon);
  test(fabs(2. * extent[2] - newExtent[2]) < epsilon);
  test(fabs(2. * extent[3] - newExtent[3]) < epsilon);
  test(fabs(extent[4] - newExtent[4]) < epsilon);
  test(fabs(extent[5] - newExtent[5]) < epsilon);

  return 0;
}

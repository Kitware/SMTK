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

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/ReadMesh.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/ExtractByDihedralAngle.h"
#include "smtk/mesh/operators/ExtractSkin.h"
#include "smtk/mesh/operators/Import.h"
#include "smtk/mesh/resource/Selection.h"
#include "smtk/mesh/utility/Create.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace
{
// SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

// A numerical tolerance
const double epsilon = 1.e-8;
} // namespace

// Import a tetrahedralized cube, extract its shell, select a cell on the shell
// to run ExtractByDihedralAngle, check that the points in the resulting
// extraction are coplanar.
int TestExtractByDihedralAngle(int, char*[])
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

  // Extract the tetrahedra cube's shell and remove its tetrahedra
  {
    smtk::mesh::MeshSet tetrahedra = resource->meshes();
    smtk::operation::Operation::Ptr extractSkin = smtk::mesh::ExtractSkin::create();
    extractSkin->parameters()->associate(smtk::mesh::Component::create(tetrahedra));
    smtk::operation::Operation::Result result = extractSkin->operate();
    test(
      result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Extract skin failed");

    auto trianglesComponent =
      std::dynamic_pointer_cast<smtk::mesh::Component>(result->findComponent("created")->value());
    smtk::mesh::MeshSet triangles = trianglesComponent->mesh();

    resource->removeMeshes(tetrahedra);

    test(resource->meshes().size() == 1);
  }

  // Run ExtractByDihedralAngle using the first triangle cell as input
  {
    smtk::mesh::HandleRange firstCell;
    firstCell.insert(*(smtk::mesh::rangeElementsBegin(resource->meshes().cells().range())));

    smtk::mesh::CellSet cellset(resource, firstCell);

    smtk::operation::Operation::Ptr extractByDihedralAngle =
      smtk::mesh::ExtractByDihedralAngle::create();

    // Create a mesh selection
    smtk::mesh::Selection::Ptr selection = smtk::mesh::Selection::create(cellset);
    extractByDihedralAngle->parameters()->associate(selection);
    extractByDihedralAngle->operate();
  }

  // Test that the points in the extracted mesh all lie in a plane
  {
    // Test that there are only two meshsets (the temporary meshset should have
    // been removed)
    test(resource->meshes().size() == 2);

    // The extracted mesh is the meshset with fewer cells
    smtk::mesh::MeshSet extractedMesh = resource->meshes();
    for (std::size_t i = 0; i < resource->meshes().size(); i++)
    {
      if (resource->meshes().subset(i).cells().size() < extractedMesh.cells().size())
      {
        extractedMesh = resource->meshes().subset(i);
      }
    }

    // Check that there are enough points in the extracted meshset to test for
    // coplanarity
    test(extractedMesh.points().size() > 3);

    // Access the extracted point coordinates
    std::vector<double> points(3 * extractedMesh.points().size());
    extractedMesh.points().get(points);

    // Construct a point <p0> on the plane and the plane's normal <n>
    std::array<double, 3> p0 = { points[0], points[1], points[2] };
    std::array<double, 3> p1 = { points[3], points[4], points[5] };
    std::array<double, 3> p2 = { points[6], points[7], points[8] };

    std::array<double, 3> v1 = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
    std::array<double, 3> v2 = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };

    std::array<double, 3> n = { v1[1] * v2[2] - v1[2] * v2[1],
                                v1[2] * v2[0] - v1[0] * v2[2],
                                v1[0] * v2[1] - v1[1] * v2[0] };

    double magnitude = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    for (std::size_t i = 0; i < n.size(); i++)
    {
      n[i] /= magnitude;
    }

    // For each subsequent point, check that the point lies on the plane
    // described by <p0> and <n>
    for (std::size_t i = 3; i < extractedMesh.points().size(); i++)
    {
      double* p = &points[3 * i];

      std::array<double, 3> v = { p[0] - p0[0], p[1] - p0[1], p[2] - p0[2] };

      double dot = v[0] * n[0] + v[1] * n[1] + v[2] * n[2];

      test(fabs(dot) < epsilon, "Extracted point does not lie in the plane");
    }
  }

  return 0;
}

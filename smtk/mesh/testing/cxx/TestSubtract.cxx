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
#include "smtk/mesh/operators/Subtract.h"
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
// to run ExtractByDihedralAngle, subtract the result from the shell, check that
// the subtraction was successful.
int TestSubtract(int, char*[])
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
  smtk::mesh::Component::Ptr shell;
  {
    smtk::mesh::MeshSet tetrahedra = resource->meshes();
    smtk::operation::Operation::Ptr extractSkin = smtk::mesh::ExtractSkin::create();
    extractSkin->parameters()->associate(smtk::mesh::Component::create(tetrahedra));
    smtk::operation::Operation::Result result = extractSkin->operate();
    test(
      result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Extract skin failed");

    shell =
      std::dynamic_pointer_cast<smtk::mesh::Component>(result->findComponent("created")->value());

    resource->removeMeshes(tetrahedra);

    test(resource->meshes().size() == 1);
  }

  // Run ExtractByDihedralAngle using the first triangle cell as input
  smtk::mesh::Component::Ptr extracted;
  {
    smtk::mesh::HandleRange firstCell;
    firstCell.insert(*(smtk::mesh::rangeElementsBegin(resource->meshes().cells().range())));

    smtk::mesh::CellSet cellset(resource, firstCell);

    smtk::operation::Operation::Ptr extractByDihedralAngle =
      smtk::mesh::ExtractByDihedralAngle::create();

    // Create a mesh selection
    smtk::mesh::Selection::Ptr selection = smtk::mesh::Selection::create(cellset);
    extractByDihedralAngle->parameters()->associate(selection);

    smtk::operation::Operation::Result result = extractByDihedralAngle->operate();
    test(
      result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Extract by dihedral angle failed");

    extracted =
      std::dynamic_pointer_cast<smtk::mesh::Component>(result->findComponent("created")->value());
  }

  // Subtract the selection from the shell
  smtk::mesh::Component::Ptr differenceMesh;
  {
    smtk::operation::Operation::Ptr subtract = smtk::mesh::Subtract::create();
    subtract->parameters()->associate(shell);
    subtract->parameters()->findComponent("subtrahend")->setValue(extracted);

    smtk::operation::Operation::Result result = subtract->operate();
    test(
      result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "subtract failed");

    differenceMesh =
      std::dynamic_pointer_cast<smtk::mesh::Component>(result->findComponent("created")->value());
  }

  smtk::mesh::HandleRange minuend = shell->mesh().cells().range();
  smtk::mesh::HandleRange subtrahend = extracted->mesh().cells().range();
  smtk::mesh::HandleRange difference = differenceMesh->mesh().cells().range();

  // Ensure that none of the ranges are empty
  test(!minuend.empty() && !subtrahend.empty() && !difference.empty());

  // Ensure that the subtrahend and difference have no intersection
  test((subtrahend & difference).empty());

  // Ensure that the sum of the subtrahend and difference is the minuend
  test(smtk::mesh::rangesEqual(minuend, subtrahend + difference));

  return 0;
}

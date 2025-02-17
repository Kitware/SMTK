//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/extension/vtk/operators/ExportFaceset.h"
#include "smtk/extension/vtk/operators/Registrar.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/plugin/Manager.txx"
#include "smtk/plugin/Registry.h"

#include "smtk/session/mesh/Registrar.h"
#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Session.h"
#include "smtk/session/mesh/operators/CreateUniformGrid.h"

#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkPLYReader.h"
#include "vtkPolyData.h"
#include "vtkSTLReader.h"
#include "vtkSmartPointer.h"

smtk::resource::ResourcePtr ConstructGrid(smtk::operation::Manager::Ptr operationManager)
{
  smtk::operation::Operation::Ptr createGridOp =
    operationManager->create<smtk::session::mesh::CreateUniformGrid>();

  if (!createGridOp)
  {
    std::cerr << "Couldn't create \"create uniform grid\" operator" << std::endl;
    return nullptr;
  }

  createGridOp->parameters()->findString("dimension")->setValue("3");
  createGridOp->parameters()->findDouble("size3d")->setValue(0, 2);

  createGridOp->parameters()->findInt("discretization3d")->setValue(0, 5);
  createGridOp->parameters()->findInt("discretization3d")->setValue(1, 5);
  createGridOp->parameters()->findInt("discretization3d")->setValue(2, 5);

  smtk::operation::Operation::Result createGridOpResult = createGridOp->operate();

  if (
    createGridOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"create uniform grid\" operation failed\n";
    return nullptr;
  }
  return createGridOpResult->findResource("resourcesCreated")->value(0);
}

void verifySTL(const std::string& filename, const int vertexCount, const int cellCount)
{
  auto fileExt = filename.substr(filename.size() - 4, 4);
  std::for_each(fileExt.begin(), fileExt.end(), [](char& c) { c = std::tolower(c); });
  vtkSmartPointer<vtkAbstractPolyDataReader> reader;
  if (fileExt == ".stl")
  {
    reader = vtkSmartPointer<vtkSTLReader>::New();
  }
  else if (fileExt == ".obj")
  {
    reader = vtkSmartPointer<vtkOBJReader>::New();
  }
  else if (fileExt == ".ply")
  {
    reader = vtkSmartPointer<vtkPLYReader>::New();
  }
  reader->SetFileName(filename.c_str());
  reader->Update();
  auto* pd = reader->GetOutput();
  test(pd->GetNumberOfPoints() == vertexCount, "Vertex count mismatch");
  test(pd->GetNumberOfCells() == cellCount, "Cell count mismatch");
}

void runTest(const std::string gridVolumeFile)
{
  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();
  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();
  // Create a geometry manager
  smtk::geometry::Manager::Ptr geometryManager = smtk::geometry::Manager::create();

  smtk::plugin::Manager::instance()->registerPluginsTo(resourceManager);
  smtk::plugin::Manager::instance()->registerPluginsTo(operationManager);
  smtk::plugin::Manager::instance()->registerPluginsTo(geometryManager);

  auto operationRegistry =
    smtk::plugin::addToManagers<smtk::operation::Registrar>(operationManager);
  auto geomRegistry =
    smtk::plugin::addToManagers<smtk::extension::vtk::operators::Registrar>(operationManager);
  auto meshRegistry = smtk::plugin::addToManagers<smtk::session::mesh::Registrar>(
    resourceManager, operationManager, geometryManager);
  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::geometry::ExportFaceset::Ptr exportOp =
    operationManager->create<smtk::geometry::ExportFaceset>();
  smtk::operation::Operation::Result exportOpResult;

  if (auto grid = ConstructGrid(operationManager))
  {
    exportOp->parameters()->findFile("filename")->setValue(gridVolumeFile);

    // Export Volume component
    auto volumeComponent = grid->filter("Volume");
    test(volumeComponent.size() == 1, "Cannot find the volume component in the input.");

    exportOp->parameters()->associate(*volumeComponent.begin());
    exportOpResult = exportOp->operate();

    test(
      exportOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Export volume failed");

    verifySTL(gridVolumeFile, 152, 300);

    // Export Face components
    auto faceComponents = grid->filter("Face");
    test(faceComponents.size() == 6, "Cannot find the face components in the input.");

    int cnt = 0;
    for (auto c = faceComponents.begin(); c != faceComponents.end(); ++c, ++cnt)
    {
      const std::string gridFaceFile = std::string("ExportFacesetTest-grid-face-") +
        std::to_string(cnt) + std::string("-output.stl");
      auto operation = operationManager->create<smtk::geometry::ExportFaceset>();
      operation->parameters()->findFile("filename")->setValue(gridFaceFile);
      operation->parameters()->associate(*c);
      auto result = operation->operate();
      test(
        result->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
        "Export face failed");
      verifySTL(gridFaceFile, 36, 50);
    }
  }
}

int main(/*int argc, char* argv[]*/)
{
  std::string gridVolumeFile = "ExportFacesetTest-grid-volume-output.stl";
  runTest(gridVolumeFile);
  gridVolumeFile = "ExportFacesetTest-grid-volume-output.obj";
  runTest(gridVolumeFile);
  gridVolumeFile = "ExportFacesetTest-grid-volume-output.ply";
  runTest(gridVolumeFile);

  return 0;
}

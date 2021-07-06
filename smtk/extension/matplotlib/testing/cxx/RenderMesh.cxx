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
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/common/PythonInterpreter.h"

#include "smtk/extension/matplotlib/Registrar.h"

#include "smtk/io/ReadMesh.h"

#include "smtk/model/DefaultSession.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/operation/Operation.h"

#include "smtk/plugin/Registry.h"

#include <fstream>
#include <streambuf>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include "smtkRegressionTestImage.h"
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkPNGReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTesting.h>

namespace
{
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
} // namespace

// Demonstrate/test the ability to call python SMTK operators within the C++
// environment.
int RenderMesh(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register matplotlib operators to the operation manager
  auto registry =
    smtk::plugin::addToManagers<smtk::extension::matplotlib::Registrar>(operationManager);

  // Access a 2-dimensional mesh with interesting z-features from the
  // data directory
  std::string input_path(data_root);
  input_path += "/mesh/2d/warpedMesh.h5m";

  smtk::mesh::ResourcePtr c = smtk::mesh::Resource::create();
  smtk::io::readMesh(input_path, c);

  if (!c || !c->isValid())
  {
    std::cerr << "Could not read mesh from " << input_path << std::endl;
    return 1;
  }

  // Construct a "render mesh" operator
  smtk::operation::Operation::Ptr op =
    operationManager->create("smtk.extension.matplotlib.render_mesh.RenderMesh");

  if (!op)
  {
    std::cerr << "Couldn't create \"render mesh\" operator" << std::endl;
    return 1;
  }

  // Set the mesh to be rendered
  op->parameters()->associate(smtk::mesh::Component::create(c->meshes()));

  // Set the file path for the rendered image
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".png";
  op->parameters()->findFile("filename")->setValue(write_path);

  // Execute the operator
  smtk::operation::Operation::Result result = op->operate();

  // Confirm that the operator succeeded
  if (
    result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "render mesh operator failed\n";
    return 1;
  }

  // Read the image
  vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
  reader->SetFileName(write_path.c_str());

  // Visualize
  vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
  actor->GetMapper()->SetInputConnection(reader->GetOutputPort());

  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(actor);
  renderer->ResetCamera();

  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  // Compare the resulting image with the baseline
  if (
    smtkRegressionTestImage(renderWindow, 10., "matplotlib_rendered_mesh.png") !=
    vtkTesting::PASSED)
  {
    return 1;
  }

  // Remove the generated file
  cleanup(write_path);

  return 0;
}

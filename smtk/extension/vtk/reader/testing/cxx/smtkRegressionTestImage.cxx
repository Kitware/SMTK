//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/reader/testing/cxx/smtkRegressionTestImage.h"
#include <vtksys/SystemTools.hxx>

// This function returns vtkTesting::PASSED on success
vtkIdType smtkRegressionTestImage(vtkRenderWindow* rw, const double thresh,
  const std::string& fileName, const std::string& fileType)
{
  // Append the testing temporary directory, the baseline image to the commandline arguments
  std::vector<std::string> args;
  args.push_back("-V");
  std::string baselineDirectory(SMTK_DATA_DIR);
  baselineDirectory = baselineDirectory + "/baseline/smtk/vtk/";
  std::string image =
    baselineDirectory + vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName) + fileType;
  args.push_back(image);
  args.push_back("-T");
  args.push_back(SMTK_SCRATCH_DIR);

  std::cout << "Commandline arguments passed to RegressionTestAndCaptureOutput:" << std::endl;
  vtkNew<vtkTesting> testing;
  for (std::vector<std::string>::size_type i = 0; i < args.size(); ++i)
  {
    std::cout << args[i] << " ";
    testing->AddArgument(args[i].c_str());
  }
  std::cout << std::endl;

  if (testing->IsInteractiveModeSpecified())
  {
    return vtkTesting::DO_INTERACTOR;
  }

  if (testing->IsValidImageSpecified())
  {
    testing->SetRenderWindow(rw);
    return testing->RegressionTestAndCaptureOutput(thresh, cout);
  }
  return vtkTesting::NOT_RUN;
}

vtkIdType runCMBGeometryReaderTest(const std::string& fileName, const double thresh)
{
  vtkNew<vtkCMBGeometryReader> reader;
  reader->SetFileName(fileName.c_str());
  reader->Update();
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(reader->GetOutputPort());
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper.GetPointer());
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(200, 200);
  renderWindow->SetMultiSamples(0);
  renderWindow->AddRenderer(renderer.GetPointer());
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow.GetPointer());
  renderer->AddActor(actor.GetPointer());
  renderWindow->Render();
  return smtkRegressionTestImage(renderWindow.GetPointer(), thresh, fileName);
}

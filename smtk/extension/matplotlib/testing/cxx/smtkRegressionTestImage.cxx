//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/matplotlib/testing/cxx/smtkRegressionTestImage.h"
#include <vtksys/SystemTools.hxx>

// This function returns vtkTesting::PASSED on success
vtkIdType smtkRegressionTestImage(
  vtkRenderWindow* rw,
  const double thresh,
  const std::string& fileName,
  const std::string& fileType)
{
  // Append the testing temporary directory, the baseline image to the commandline arguments
  std::vector<std::string> args;
  args.emplace_back("-V");
  std::string baselineDirectory(SMTK_DATA_DIR);
  baselineDirectory = baselineDirectory + "/baseline/smtk/vtk/";
  std::string image =
    baselineDirectory + vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName) + fileType;
  args.push_back(image);
  args.emplace_back("-T");
  args.emplace_back(SMTK_SCRATCH_DIR);

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

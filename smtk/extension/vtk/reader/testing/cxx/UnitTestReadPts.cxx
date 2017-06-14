//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtkRegressionTestImage.h"

int UnitTestReadPts(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  std::string file = SMTK_DATA_DIR;
  file = file + "/point_cloud/cylinder.pts";
  if (runCMBGeometryReaderTest(file) != vtkTesting::PASSED)
  {
    return 1;
  }
  std::cout << "Test for reading " << file << " with CMBGeometryReader succeeded" << std::endl;
  return 0;
}

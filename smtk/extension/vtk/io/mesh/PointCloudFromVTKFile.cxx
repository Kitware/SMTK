//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/io/mesh/PointCloudFromVTKFile.h"

#include "smtk/extension/vtk/io/ImportAsVTKData.h"

#include "smtk/model/AuxiliaryGeometry.h"

#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkUniformGrid.h"

#include <vtksys/SystemTools.hxx>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

namespace
{
bool registered = PointCloudFromVTKFile::registerClass();
}

bool PointCloudFromVTKFile::valid(const std::string& fileName) const
{
  smtk::extension::vtk::io::ImportAsVTKData importAsVTKData;
  return importAsVTKData.valid(fileName);
}

smtk::mesh::PointCloud PointCloudFromVTKFile::operator()(const std::string& fileName)
{
  smtk::extension::vtk::io::ImportAsVTKData importAsVTKData;
  auto externalData = vtkDataSet::SafeDownCast(importAsVTKData(fileName));
  if (!externalData)
  {
    // Something went wrong and we have no vtkDataSet.
    throw std::invalid_argument("File cannot be read.");
  }

  std::function<std::array<double, 3>(std::size_t)> coordinates = [&](std::size_t i) {
    double pt[3];
    externalData->GetPoint(i, pt);
    return std::array<double, 3>({ { pt[0], pt[1], 0. } });
  };

  std::function<double(std::size_t)> data;

  // Check for elevation data. If it exists, use it. Otherwise, just use the
  // z-coordinate of the data,
  // TODO: no magic keywords!
  vtkDataArray* elevationData = externalData->GetPointData()->GetScalars("Elevation");
  if (elevationData)
  {
    data = [=](std::size_t i) { return elevationData->GetTuple1(i); };
  }
  else
  {
    data = [&](std::size_t i) {
      double pt[3];
      externalData->GetPoint(i, pt);
      return pt[2];
    };
  }

  std::function<bool(std::size_t)> valid = [](std::size_t /*unused*/) { return true; };

  return smtk::mesh::PointCloud(externalData->GetNumberOfPoints(), coordinates, data, valid);
}
} // namespace mesh
} // namespace vtk
} // namespace extension
} // namespace smtk

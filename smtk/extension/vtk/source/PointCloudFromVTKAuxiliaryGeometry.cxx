//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/source/PointCloudFromVTKAuxiliaryGeometry.h"

#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

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
bool registered = PointCloudFromVTKAuxiliaryGeometry::registerClass();
}

bool PointCloudFromVTKAuxiliaryGeometry::valid(const smtk::model::AuxiliaryGeometry& auxGeom) const
{
  auto loader = vtkAuxiliaryGeometryExtension::create();
  std::vector<double> bbox(6);

  // TODO: why does canHandleAuxiliaryGeometry not take a const AuxiliaryGeometry?
  smtk::model::AuxiliaryGeometry nonConstAuxGeom(auxGeom);

  return (loader && loader->canHandleAuxiliaryGeometry(nonConstAuxGeom, bbox));
}

smtk::mesh::PointCloud PointCloudFromVTKAuxiliaryGeometry::operator()(
  const smtk::model::AuxiliaryGeometry& auxGeom)
{
  // Convert the auxiliary geometry from a file name to a vtkDataset
  vtkDataSet* externalData = nullptr;
  auto loader = vtkAuxiliaryGeometryExtension::create();
  std::vector<double> bbox(6);

  smtk::model::AuxiliaryGeometry nonConstAuxGeom(auxGeom);

  if (loader && loader->canHandleAuxiliaryGeometry(nonConstAuxGeom, bbox))
  {
    externalData =
      vtkDataSet::SafeDownCast(vtkAuxiliaryGeometryExtension::fetchCachedGeometry(auxGeom));
  }

  if (!externalData)
  {
    // Something went wrong and we have no vtkDataSet.
    throw std::invalid_argument("Auxiliary geometry cannot be read.");
  }

  std::function<std::array<double, 3>(std::size_t)> coordinates = [=](std::size_t i) {
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
    data = [=](std::size_t i) {
      double pt[3];
      externalData->GetPoint(i, pt);
      return pt[2];
    };
  }

  return smtk::mesh::PointCloud(externalData->GetNumberOfPoints(), coordinates, data);
}
} // namespace mesh
} // namespace vtk
} // namespace extension
} // namespace smtk

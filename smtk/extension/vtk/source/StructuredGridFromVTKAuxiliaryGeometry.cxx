//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/source/StructuredGridFromVTKAuxiliaryGeometry.h"

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
bool registered = StructuredGridFromVTKAuxiliaryGeometry::registerClass();
}

bool StructuredGridFromVTKAuxiliaryGeometry::valid(
  const smtk::model::AuxiliaryGeometry& auxGeom) const
{
  auto loader = vtkAuxiliaryGeometryExtension::create();
  std::vector<double> bbox(6);

  // TODO: why does canHandleAuxiliaryGeometry not take a const AuxiliaryGeometry?
  smtk::model::AuxiliaryGeometry nonConstAuxGeom(auxGeom);

  return (loader && loader->canHandleAuxiliaryGeometry(nonConstAuxGeom, bbox));
}

smtk::mesh::StructuredGrid StructuredGridFromVTKAuxiliaryGeometry::operator()(
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
    throw std::invalid_argument("File cannot be read.");
  }

  smtk::mesh::StructuredGrid structuredgrid;

  if (vtkImageData* imageInput = vtkImageData::SafeDownCast(externalData))
  {
    structuredgrid = smtk::mesh::StructuredGrid(
      imageInput->GetExtent(),
      imageInput->GetOrigin(),
      imageInput->GetSpacing(),
      [=](int i, int j) { return imageInput->GetScalarComponentAsDouble(i, j, 0, 0); });
  }
  else if (vtkUniformGrid* gridInput = vtkUniformGrid::SafeDownCast(externalData))
  {
    structuredgrid = smtk::mesh::StructuredGrid(
      gridInput->GetExtent(),
      gridInput->GetOrigin(),
      gridInput->GetSpacing(),
      [=](int i, int j) { return gridInput->GetScalarComponentAsDouble(i, j, 0, 0); },
      [=](int i, int j) {
        int pos[3] = { i, j, 0 };
        return gridInput->IsPointVisible(
                 vtkStructuredData::ComputePointIdForExtent(gridInput->GetExtent(), pos)) != 0;
      });
  }
  else
  {
    throw std::invalid_argument("File does not contain structured data.");
  }

  return structuredgrid;
}
} // namespace mesh
} // namespace vtk
} // namespace extension
} // namespace smtk

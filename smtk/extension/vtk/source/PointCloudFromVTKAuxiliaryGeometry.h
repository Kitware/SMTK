//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extensions_vtk_source_PointCloudFromVTK_h
#define smtk_extensions_vtk_source_PointCloudFromVTK_h
#ifndef __VTK_WRAP__

#include "smtk/AutoInit.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h"

#include "smtk/common/Generator.h"
#include "smtk/mesh/interpolation/PointCloud.h"
#include "smtk/mesh/interpolation/PointCloudGenerator.h"

#include <string>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

/// A GeneratorType for creating PointClouds from VTK auxiliary geometry. This
/// class extends smtk::mesh::PointCloudGenerator.
class VTKSMTKSOURCEEXT_EXPORT PointCloudFromVTKAuxiliaryGeometry
  : public smtk::common::GeneratorType<
      smtk::model::AuxiliaryGeometry,
      smtk::mesh::PointCloud,
      PointCloudFromVTKAuxiliaryGeometry>
{
public:
  bool valid(const smtk::model::AuxiliaryGeometry& auxGeom) const override;

  smtk::mesh::PointCloud operator()(const smtk::model::AuxiliaryGeometry& auxGeom) override;
};
} // namespace mesh
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif // __VTK_WRAP__
#endif

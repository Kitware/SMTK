//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extensions_vtk_mesh_PointCloudFromVTK_h
#define __smtk_extensions_vtk_mesh_PointCloudFromVTK_h

#include "smtk/AutoInit.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/io/IOVTKExports.h"

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

/// A GeneratorType for creating PointClouds from VTK files. This class extends
/// smtk::mesh::PointCloudGenerator.
class SMTKIOVTK_EXPORT PointCloudFromVTKFile
  : public smtk::common::GeneratorType<std::string, smtk::mesh::PointCloud, PointCloudFromVTKFile>
{
public:
  bool valid(const std::string& file) const override;

  smtk::mesh::PointCloud operator()(const std::string& file) override;
};
} // namespace mesh
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif

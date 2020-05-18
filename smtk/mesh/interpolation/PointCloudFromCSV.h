//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_PointCloudFromCSV_h
#define __smtk_mesh_PointCloudFromCSV_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Generator.h"
#include "smtk/mesh/interpolation/PointCloud.h"

#include <string>

namespace smtk
{
namespace mesh
{

/// A GeneratorType for creating PointClouds from CSV files. This class extends
/// smtk::mesh::PointCloudGenerator.
class SMTKCORE_EXPORT PointCloudFromCSV
  : public smtk::common::GeneratorType<std::string, PointCloud, PointCloudFromCSV>
{
public:
  bool valid(const std::string& file) const override;

  smtk::mesh::PointCloud operator()(const std::string& file) override;
};
} // namespace mesh
} // namespace smtk

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_PointCloudGenerator_h
#define smtk_mesh_PointCloudGenerator_h

#include "smtk/CoreExports.h"
#include "smtk/common/Generator.h"
#include "smtk/mesh/interpolation/PointCloud.h"

#include <string>

template class SMTKCORE_EXPORT smtk::common::Generator<std::string, smtk::mesh::PointCloud>;
template class SMTKCORE_EXPORT
  smtk::common::Generator<smtk::model::AuxiliaryGeometry, smtk::mesh::PointCloud>;

namespace smtk
{
namespace model
{
class AuxiliaryGeometry;
}

/// A generator for PointClouds. PointCloudGenerator accepts as input (a) a
/// string describing a data file or (b) an auxiliary geometry instance. This
/// class is extended by classes that inherit from GeneratorType (see
/// smtk::common::Generator).
namespace mesh
{

class SMTKCORE_EXPORT PointCloudGenerator
  : public smtk::common::Generator<std::string, PointCloud>
  , public smtk::common::Generator<smtk::model::AuxiliaryGeometry, PointCloud>
{
public:
  using smtk::common::Generator<std::string, PointCloud>::operator();
  using smtk::common::Generator<std::string, PointCloud>::valid;
  using smtk::common::Generator<smtk::model::AuxiliaryGeometry, PointCloud>::operator();
  using smtk::common::Generator<smtk::model::AuxiliaryGeometry, PointCloud>::valid;

  ~PointCloudGenerator() override;
};
} // namespace mesh
} // namespace smtk

#endif

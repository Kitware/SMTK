//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_PointCloudGenerator_h
#define __smtk_mesh_PointCloudGenerator_h

#include "smtk/CoreExports.h"
#include "smtk/common/Generator.h"
#include "smtk/mesh/interpolation/PointCloud.h"

#include <string>

#ifndef smtkCore_EXPORTS
extern
#endif
  template class smtk::common::Generator<std::string, smtk::mesh::PointCloud>;

#ifndef smtkCore_EXPORTS
extern
#endif
  template class smtk::common::Generator<smtk::model::AuxiliaryGeometry, smtk::mesh::PointCloud>;

namespace smtk
{
namespace model
{
class AuxiliaryGeometry;
}

namespace mesh
{

class SMTKCORE_EXPORT PointCloudGenerator
  : public smtk::common::Generator<std::string, PointCloud>,
    public smtk::common::Generator<smtk::model::AuxiliaryGeometry, PointCloud>
{
public:
  using smtk::common::Generator<std::string, PointCloud>::operator();
  using smtk::common::Generator<smtk::model::AuxiliaryGeometry, PointCloud>::operator();

  virtual ~PointCloudGenerator();
};
}
}

#endif

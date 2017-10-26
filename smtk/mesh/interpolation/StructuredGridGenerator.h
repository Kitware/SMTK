//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_StructuredGridGenerator_h
#define __smtk_mesh_StructuredGridGenerator_h

#include "smtk/CoreExports.h"
#include "smtk/common/Generator.h"
#include "smtk/mesh/interpolation/StructuredGrid.h"

#include <string>

#ifndef smtkCore_EXPORTS
extern
#endif
  template class smtk::common::Generator<std::string, smtk::mesh::StructuredGrid>;

#ifndef smtkCore_EXPORTS
extern
#endif
  template class smtk::common::Generator<smtk::model::AuxiliaryGeometry,
    smtk::mesh::StructuredGrid>;

namespace smtk
{
namespace model
{
class AuxiliaryGeometry;
}

namespace mesh
{

class SMTKCORE_EXPORT StructuredGridGenerator
  : public smtk::common::Generator<std::string, StructuredGrid>,
    public smtk::common::Generator<smtk::model::AuxiliaryGeometry, StructuredGrid>
{
public:
  using smtk::common::Generator<std::string, StructuredGrid>::operator();
  using smtk::common::Generator<smtk::model::AuxiliaryGeometry, StructuredGrid>::operator();

  virtual ~StructuredGridGenerator();
};
}
}

#endif

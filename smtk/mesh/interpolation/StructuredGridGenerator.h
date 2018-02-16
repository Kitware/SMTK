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

template class SMTKCORE_EXPORT smtk::common::Generator<std::string, smtk::mesh::StructuredGrid>;
template class SMTKCORE_EXPORT
  smtk::common::Generator<smtk::model::AuxiliaryGeometry, smtk::mesh::StructuredGrid>;

namespace smtk
{
namespace model
{
class AuxiliaryGeometry;
}

/// A generator for StructuredGrids. StructuredGridGenerator accepts as input
/// (a) a  string describing a data file or (b) an auxiliary geometry instance.
/// This  class is extended by classes that inherit from GeneratorType (see
/// smtk::common::Generator).
namespace mesh
{

class SMTKCORE_EXPORT StructuredGridGenerator
  : public smtk::common::Generator<std::string, StructuredGrid>,
    public smtk::common::Generator<smtk::model::AuxiliaryGeometry, StructuredGrid>
{
public:
  using smtk::common::Generator<std::string, StructuredGrid>::operator();
  using smtk::common::Generator<std::string, StructuredGrid>::valid;
  using smtk::common::Generator<smtk::model::AuxiliaryGeometry, StructuredGrid>::operator();
  using smtk::common::Generator<smtk::model::AuxiliaryGeometry, StructuredGrid>::valid;

  virtual ~StructuredGridGenerator();
};
}
}

#endif

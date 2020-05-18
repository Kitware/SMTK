//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extensions_vtk_source_StructuredGridFromVTKAuxiliaryGeometry_h
#define __smtk_extensions_vtk_source_StructuredGridFromVTKAuxiliaryGeometry_h
#ifndef __VTK_WRAP__

#include "smtk/AutoInit.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h"

#include "smtk/mesh/interpolation/StructuredGrid.h"
#include "smtk/mesh/interpolation/StructuredGridGenerator.h"

#include <string>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

/// A GeneratorType for creating StructuredGrids from VTK auxiliary geometry.
/// This class extends smtk::mesh::StructuredGridGenerator.
class VTKSMTKSOURCEEXT_EXPORT StructuredGridFromVTKAuxiliaryGeometry
  : public smtk::common::GeneratorType<
      smtk::model::AuxiliaryGeometry,
      smtk::mesh::StructuredGrid,
      StructuredGridFromVTKAuxiliaryGeometry>
{
public:
  bool valid(const smtk::model::AuxiliaryGeometry&) const override;

  smtk::mesh::StructuredGrid operator()(const smtk::model::AuxiliaryGeometry&) override;
};
} // namespace mesh
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif // __VTK_WRAP__
#endif

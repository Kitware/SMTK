//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extensions_vtk_io_mesh_StructuredGridFromVTKFile_h
#define __smtk_extensions_vtk_io_mesh_StructuredGridFromVTKFile_h

#include "smtk/AutoInit.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/io/IOVTKExports.h"

#include "smtk/common/Generator.h"
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

/// A GeneratorType for creating StructuredGrids from VTK files. This class
/// extends smtk::mesh::StructuredGridGenerator.
class SMTKIOVTK_EXPORT StructuredGridFromVTKFile
  : public smtk::common::GeneratorType<std::string, smtk::mesh::StructuredGrid,
      StructuredGridFromVTKFile>
{
public:
  bool valid(const std::string&) const override;

  smtk::mesh::StructuredGrid operator()(const std::string&) override;
};
}
}
}
}

#endif

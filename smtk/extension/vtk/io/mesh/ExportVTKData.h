//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef smtk_extension_vtk_io_mesh_ExportVTKData_h
#define smtk_extension_vtk_io_mesh_ExportVTKData_h

#include "smtk/extension/vtk/io/IOVTKExports.h"
//forward declarers for Manager and Resource
#include "smtk/PublicPointerDefs.h"

#include <string>

//forward declare vtk classes
class vtkPolyData;
class vtkUnstructuredGrid;

namespace smtk
{
namespace mesh
{
class MeshSet;
}
} // namespace smtk

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{
namespace mesh
{

//Export an smtk::mesh::resource to a VTK data set.
class SMTKIOVTK_EXPORT ExportVTKData
{
public:
  explicit ExportVTKData();

  ExportVTKData(const ExportVTKData& other) = delete;
  ExportVTKData& operator=(const ExportVTKData& other) = delete;

  //Export a resource as a VTK xml polydata or xml unstructured grid file
  //(determined by the file name suffix .vtp or .vtu).
  bool operator()(
    const std::string& filename,
    smtk::mesh::ResourcePtr resource,
    std::string domainPropertyName) const;

  //Export a meshset as a VTK xml polydata or xml unstructured grid file
  //(determined by the file name suffix .vtp or .vtu).
  bool operator()(
    const std::string& filename,
    const smtk::mesh::MeshSet& meshset,
    std::string domainPropertyName) const;

  //Export the highest dimension cells of a mesh set to polydata. If the highest
  //dimension is Dims3, export its shell.
  void operator()(
    const smtk::mesh::MeshSet& meshset,
    vtkPolyData* pd,
    std::string domainPropertyName = std::string()) const;

  //Export a mesh set to an unstructured grid.
  void operator()(
    const smtk::mesh::MeshSet& meshset,
    vtkUnstructuredGrid* ug,
    std::string domainPropertyName = std::string()) const;
};
} // namespace mesh
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif //__smtk_extension_vtk_io_mesh_ExportVTKData_h

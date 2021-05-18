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
#ifndef __smtk_extension_vtk_io_mesh_ImportVTKData_h
#define __smtk_extension_vtk_io_mesh_ImportVTKData_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/io/IOVTKExports.h"

#include <string>

//forward declare vtk classes
class vtkDataSet;

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

//Import a VTK data set to a smtk::mesh::resource.
//Currently we only support creating a new database from a vtk data set.
class SMTKIOVTK_EXPORT ImportVTKData
{
public:
  explicit ImportVTKData();

  ImportVTKData(const ImportVTKData& other) = delete;
  ImportVTKData& operator=(const ImportVTKData& other) = delete;

  //Import a VTK dataset unstructured grid file (legacy or xml) as a
  //resource. Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::ResourcePtr operator()(
    const std::string& filename,
    const smtk::mesh::InterfacePtr& interface,
    std::string domainPropertyName) const;

  //Import a VTK dataset or unstructured grid file (legacy or xml) into an
  //existing resource. Optionally specify the cell property name to be used to
  //split the mesh into muliple domains.
  bool operator()(
    const std::string& filename,
    smtk::mesh::ResourcePtr resource,
    std::string domainPropertyName) const;

  //Import a VTK dataset into an existing resource. Returns a meshset
  //containing the newly created cells.
  smtk::mesh::MeshSet operator()(vtkDataSet* dataset, smtk::mesh::ResourcePtr resource) const;

  //Import a VTK dataset into an existing resource and specify the
  //cell property name to be used to split the mesh into muliple domains.
  bool operator()(
    vtkDataSet* dataset,
    smtk::mesh::ResourcePtr resource,
    std::string domainPropertyName) const;

  //Import a VTK dataset as a resource.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::ResourcePtr operator()(
    vtkDataSet* dataset,
    const smtk::mesh::InterfacePtr& interface,
    std::string domainPropertyName = std::string()) const;
};
} // namespace mesh
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif //__smtk_extension_vtk_io_mesh_ImportVTKData_h

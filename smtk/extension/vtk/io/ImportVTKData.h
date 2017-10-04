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
#ifndef __smtk_extension_vtk_io_ImportVTKData_h
#define __smtk_extension_vtk_io_ImportVTKData_h

#include "smtk/extension/vtk/io/IOVTKExports.h"
//forward declarers for Manager and Collection
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
}

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

//Import a VTK data set to a smtk::mesh::collection.
//Currently we only support creating a new database from a vtk data set.
class SMTKIOVTK_EXPORT ImportVTKData
{
public:
  explicit ImportVTKData();

  //Import a VTK polydata unstructured grid file (legacy or xml) as a
  //collection. Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::CollectionPtr operator()(const std::string& filename, smtk::mesh::ManagerPtr& manager,
    std::string domainPropertyName) const;

  //Import a VTK polydata or unstructured grid file (legacy or xml) into an
  //existing collection. Optionally specify the cell property name to be used to
  //split the mesh into muliple domains.
  bool operator()(const std::string& filename, smtk::mesh::CollectionPtr collection,
    std::string domainPropertyName) const;

  //Import a VTK polydata into an existing collection. Returns a meshset
  //containing the newly created cells.
  smtk::mesh::MeshSet operator()(vtkPolyData* polydata, smtk::mesh::CollectionPtr collection) const;

  //Import a VTK polydata into an existing collection and specify the
  //cell property name to be used to split the mesh into muliple domains.
  bool operator()(vtkPolyData* polydata, smtk::mesh::CollectionPtr collection,
    std::string domainPropertyName) const;

  //Import a VTK polydata as a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::CollectionPtr operator()(vtkPolyData* polydata, smtk::mesh::ManagerPtr& manager,
    std::string domainPropertyName = std::string()) const;

  //Import a VTK unstructured grid into an existing collection. Optionally
  //specify the cell property name to be used to split the mesh into muliple
  //domains.
  bool operator()(vtkUnstructuredGrid* ugrid, smtk::mesh::CollectionPtr collection,
    std::string domainPropertyName = std::string()) const;

  //Import a VTK unstructured grid as a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::CollectionPtr operator()(vtkUnstructuredGrid* ugrid, smtk::mesh::ManagerPtr& manager,
    std::string domainPropertyName = std::string()) const;

private:
  //both are blank since we currently don't want to support copy by value
  ImportVTKData(const ImportVTKData& other);
  ImportVTKData& operator=(const ImportVTKData& other);
};
}
}
}
}

#endif //__smtk_extension_vtkToMesh_ImportVTKData_h

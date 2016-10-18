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
#ifndef __smtk_extension_vtk_io_VTKDataConverter_h
#define __smtk_extension_vtk_io_VTKDataConverter_h

#include "smtk/extension/vtk/io/IOVTKExports.h"
//forward declarers for Manager and Collection
#include "smtk/PublicPointerDefs.h"

#include <string>

//forward declare vtk classes
template <class X>
class vtkSmartPointer;
class vtkPolyData;
class vtkUnstructuredGrid;

namespace smtk {
namespace mesh {
class MeshSet;
}
}

namespace smtk {
namespace extension {
namespace vtk {
namespace io {

//Convert a VTK data set to a smtk::mesh::collection.
//Currently we only support creating a new database from a vtk data set.
//
//TODO: Allow insertion of a vtk dataset into an existing collection
class SMTKIOVTK_EXPORT VTKDataConverter
{
public:
  //Construct a VTKDataConverter
  explicit VTKDataConverter();

  //convert a VTK xml polydata or xml unstructured grid file to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::CollectionPtr operator()(const std::string& filename,
                                       smtk::mesh::ManagerPtr& manager,
                                       std::string domainPropertyName) const;

  //convert a VTK xml polydata or xml unstructured grid file to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  bool operator()(const std::string& filename,
                  smtk::mesh::CollectionPtr collection,
                  std::string domainPropertyName) const;

  //convert a polydata to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  bool operator()(vtkPolyData* polydata,
                  smtk::mesh::CollectionPtr collection,
                  std::string domainPropertyName = std::string()) const;

  //convert a polydata to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::CollectionPtr operator()(vtkPolyData* polydata,
                                       smtk::mesh::ManagerPtr& manager,
                                       std::string domainPropertyName = std::string()) const;

  //convert an unstructured grid to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  bool operator()(vtkUnstructuredGrid* ugrid,
                  smtk::mesh::CollectionPtr collection,
                  std::string domainPropertyName = std::string()) const;

  //convert an unstructured grid to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domains.
  smtk::mesh::CollectionPtr operator()(vtkUnstructuredGrid* ugrid,
                                       smtk::mesh::ManagerPtr& manager,
                                       std::string domainPropertyName = std::string()) const;

  //convert a collection into a VTK xml polydata or xml unstructured grid file.
  bool operator()(const std::string& filename,
                  smtk::mesh::CollectionPtr collection) const;

  //convert the highest dimension cells of a mesh set to polydata (starting with
  //Dims2).
  void operator()(const smtk::mesh::MeshSet& meshset, vtkPolyData* pd) const;

  //convert a mesh set to an unstructured grid.
  void operator()(const smtk::mesh::MeshSet& meshset,
                  vtkUnstructuredGrid* ug) const;

private:
  //both are blank since we currently don't want to support copy by value
  VTKDataConverter( const VTKDataConverter& other );
  VTKDataConverter& operator=( const VTKDataConverter& other );
};

}
}
}
}

#endif //__smtk_extension_vtkToMesh_VTKDataConverter_h

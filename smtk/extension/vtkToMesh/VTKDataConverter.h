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
#ifndef __smtk_extension_vtkToMesh_VTKDataConverter_h
#define __smtk_extension_vtkToMesh_VTKDataConverter_h

#include "smtk/extension/vtkToMesh/vtkToSMTKMeshExports.h"
//forward declarers for Manager and Collection
#include "smtk/PublicPointerDefs.h"

//forward declare vtk classes
class vtkPolyData;
class vtkUnstructuredGrid;

#include <string>

namespace smtk {
namespace extension {
namespace vtkToMesh {

//Convert a VTK data set to a smtk::mesh::collection.
//Currently we only support creating a new database from a vtk data set.
//
//TODO: Allow insertion of a vtk dataset into an existing collection
class VTKTOSMTKMESH_EXPORT VTKDataConverter
{
public:
  //Construct a VTKDataConverter and tie it to a manager. This means
  //that all conversion will be added as new collections to this manager
  explicit VTKDataConverter(const smtk::mesh::ManagerPtr& manager);

  //convert a polydata to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domain.
  smtk::mesh::CollectionPtr operator()(vtkPolyData* polydata,
                                       std::string domainPropertyName = std::string()) const;

  //convert an unstructured grid to a collection.
  //Optionally specify the cell property name to be used to split
  //the mesh into muliple domain.
  smtk::mesh::CollectionPtr operator()(vtkUnstructuredGrid* ugrid,
                                       std::string domainPropertyName = std::string()) const;

private:
  //both are blank since we currently don't want to support copy by value
  VTKDataConverter( const VTKDataConverter& other );
  VTKDataConverter& operator=( const VTKDataConverter& other );

  //holds a weak reference to the manager
  smtk::weak_ptr<smtk::mesh::Manager> m_manager;
};

}
}
}

#endif //__smtk_extension_vtkToMesh_VTKDataConverter_h

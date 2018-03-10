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
#ifndef __smtk_extension_vtk_io_mesh_ImportSTLFile_h
#define __smtk_extension_vtk_io_mesh_ImportSTLFile_h

#include "smtk/extension/vtk/io/IOVTKExports.h"
//forward declarers for Manager and Collection
#include "smtk/PublicPointerDefs.h"

#include <string>

//forward declare vtk classes
class vtkDataSet;

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
namespace mesh
{

//Import an STL file to a smtk::mesh::collection.
class SMTKIOVTK_EXPORT ImportSTLFile
{
public:
  explicit ImportSTLFile();

  //Import an STL file as a collection.
  smtk::mesh::CollectionPtr operator()(
    const std::string& filename, smtk::mesh::ManagerPtr& manager) const;

  bool operator()(const std::string& filename, smtk::mesh::CollectionPtr collection) const;
};
}
}
}
}
}

#endif //__smtk_extension_vtk_io_mesh_ImportSTLFile_h

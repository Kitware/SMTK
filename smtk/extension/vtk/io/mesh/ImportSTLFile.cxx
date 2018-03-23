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

#include "smtk/extension/vtk/io/mesh/ImportSTLFile.h"
#include "smtk/extension/vtk/io/mesh/ImportVTKData.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "vtkCellData.h"
#include "vtkSTLReader.h"

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
ImportSTLFile::ImportSTLFile()
{
}

smtk::mesh::CollectionPtr ImportSTLFile::operator()(
  const std::string& filename, smtk::mesh::ManagerPtr& manager) const
{
  smtk::mesh::CollectionPtr collection = manager->makeCollection();
  return this->operator()(filename, collection) ? collection : smtk::mesh::CollectionPtr();
}

bool ImportSTLFile::operator()(
  const std::string& filename, smtk::mesh::CollectionPtr collection) const
{
  vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
  reader->ScalarTagsOn();
  reader->SetFileName(filename.c_str());
  reader->Update();
  reader->GetOutput()->Register(reader);
  vtkDataSet* data = vtkDataSet::SafeDownCast(reader->GetOutput());

  ImportVTKData importVTKData;
  if (data->GetCellData()->HasArray("STLSolidLabeling"))
  {
    return importVTKData(data, collection, "STLSolidLabeling");
  }
  else
  {
    return importVTKData(data, collection, "");
  }
}
}
}
}
}
}

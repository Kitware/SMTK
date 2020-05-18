//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/source/SourceFromMesh.h"
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/mesh/core/Resource.h"

#include <cassert>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{

namespace
{
bool registered = SourceFromMesh::registerClass();
}

bool SourceFromMesh::valid(const smtk::resource::ResourcePtr& resource) const
{
  return std::dynamic_pointer_cast<smtk::mesh::Resource>(resource) != nullptr;
}

vtkSmartPointer<vtkAlgorithm> SourceFromMesh::operator()(
  const smtk::resource::ResourcePtr& resource)
{
  auto meshResource = std::static_pointer_cast<smtk::mesh::Resource>(resource);

  // The valid() call above should make certain that the static pointer cast
  // will succeed. It doesn't hurt to be cautious, though.
  assert(meshResource);

  // Create a vtkResourceMultiBlockSource for our mesh.
  auto source = vtkSmartPointer<vtkResourceMultiBlockSource>::New();
  source->SetResource(meshResource);

  return source;
}
} // namespace source
} // namespace vtk
} // namespace extension
} // namespace smtk

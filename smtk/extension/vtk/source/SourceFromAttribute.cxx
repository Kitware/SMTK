//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/source/SourceFromAttribute.h"
#include "smtk/extension/vtk/source/vtkAttributeMultiBlockSource.h"

#include "smtk/attribute/Resource.h"

#include "vtkMultiBlockDataSet.h"
#include "vtkTrivialProducer.h"

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
bool registered = SourceFromAttribute::registerClass();
}

bool SourceFromAttribute::valid(const smtk::resource::ResourcePtr& resource) const
{
  return std::dynamic_pointer_cast<smtk::attribute::Resource>(resource) != nullptr;
}

vtkSmartPointer<vtkAlgorithm> SourceFromAttribute::operator()(
  const smtk::resource::ResourcePtr& resource)
{
  auto attributeResource = std::static_pointer_cast<smtk::attribute::Resource>(resource);

  // The valid() call above should make certain that the static pointer cast
  // will succeed. It doesn't hurt to be cautious, though.
  assert(attributeResource);

  // Create a vtkAttributeMultiBlockSource for our attribute.
  auto source = vtkSmartPointer<vtkAttributeMultiBlockSource>::New();

  // Tell our multiblock source to generate data from attribute components.
  source->SetResource(attributeResource);

  return source;
}
} // namespace source
} // namespace vtk
} // namespace extension
} // namespace smtk

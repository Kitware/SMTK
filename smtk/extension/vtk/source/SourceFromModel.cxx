//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/source/SourceFromModel.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/model/Resource.h"

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
static bool registered = SourceFromModel::registerClass();
}

bool SourceFromModel::valid(const smtk::resource::ResourcePtr& resource) const
{
  return std::dynamic_pointer_cast<smtk::model::Resource>(resource) != nullptr;
}

vtkSmartPointer<vtkAlgorithm> SourceFromModel::operator()(
  const smtk::resource::ResourcePtr& resource)
{
  auto modelResource = std::static_pointer_cast<smtk::model::Resource>(resource);

  // The valid() call above should make certain that the static pointer cast
  // will succeed. It doesn't hurt to be cautious, though.
  assert(modelResource);

  // Create a vtkModelMultiBlockSource for our model.
  auto source = vtkSmartPointer<vtkModelMultiBlockSource>::New();

  // Tell our multiblock source to generate VTK polydata for model entities.
  source->SetModelResource(modelResource);

  // Also, find the first model and tell the multiblock source to render only it.
  // From dcthomp:
  // TODO: Either we need a separate representation for each model (which is
  //       IMNSHO a bad idea) or we need to change the multiblock source to work
  //       without a model. It currently generates tessellations but not all the
  //       metadata required for color-by modes.
  auto models =
    modelResource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);
  if (!models.empty())
  {
    source->SetModelEntityID(models.begin()->entity().toString().c_str());
  }
  return source;
}
}
}
}
}

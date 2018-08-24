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
static bool registered = SourceFromAttribute::registerClass();
}

bool SourceFromAttribute::valid(const smtk::resource::ResourcePtr& resource) const
{
  return std::dynamic_pointer_cast<smtk::attribute::Resource>(resource) != nullptr;
}

vtkSmartPointer<vtkAlgorithm> SourceFromAttribute::operator()(const smtk::resource::ResourcePtr&)
{
  vtkNew<vtkMultiBlockDataSet> multiblock;
  multiblock->SetNumberOfBlocks(1);

  auto source = vtkSmartPointer<vtkTrivialProducer>::New();
  source->SetOutput(multiblock);

  return source;
}
}
}
}
}

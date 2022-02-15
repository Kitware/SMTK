//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================

#include "smtk/io/Logger.h"

#include "smtk/extension/vtk/operators/ImageInspector.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/geometry/queries/BoundingBox.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/Component.h"

#include "vtkDataArray.h"
#include "vtkDataObject.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExtractSurface.h"
#include "vtkPolyData.h"

#include "smtk/extension/vtk/operators/ImageInspector_xml.h"

namespace smtk
{
namespace geometry
{

ImageInspector::Result ImageInspector::operateInternal()
{
  auto result = this->createResult(smtk::operation::Operation::Outcome::FAILED);

  // do nothing
  result->findInt("outcome")->setValue(static_cast<int>(ImageInspector::Outcome::SUCCEEDED));
  return result;
}

const char* ImageInspector::xmlDescription() const
{
  return ImageInspector_xml;
}
} // namespace geometry
} // namespace smtk

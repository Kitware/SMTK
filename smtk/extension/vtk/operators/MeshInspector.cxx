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

#include "smtk/extension/vtk/operators/MeshInspector.h"

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

#include "smtk/extension/vtk/operators/MeshInspector_xml.h"

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

namespace smtk
{
namespace geometry
{

MeshInspector::Result MeshInspector::operateInternal()
{
  auto result = this->createResult(smtk::operation::Operation::Outcome::FAILED);

  // do nothing
  result->findInt("outcome")->setValue(static_cast<int>(MeshInspector::Outcome::SUCCEEDED));
  return result;
}

const char* MeshInspector::xmlDescription() const
{
  return MeshInspector_xml;
}
} // namespace geometry
} // namespace smtk

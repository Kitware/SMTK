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
#include "smtk/extension/vtk/operators/Registrar.h"

#include "smtk/geometry/Manager.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Manager.h"

#include "smtk/extension/vtk/operators/DataSetInfoInspector.h"
#include "smtk/extension/vtk/operators/ExportFaceset.h"
#include "smtk/extension/vtk/operators/ImageInspector.h"
#include "smtk/extension/vtk/operators/MeshInspector.h"

#include "smtk/plugin/Manager.h"

#include "smtk/extension/vtk/operators/icons/export_faceset_svg.h"
#include "smtk/extension/vtk/operators/icons/image_inspector_svg.h"
#include "smtk/extension/vtk/operators/icons/mesh_inspector_svg.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace operators
{
namespace
{
using GeometryOperations = std::tuple<
  geometry::DataSetInfoInspector,
  geometry::ImageInspector,
  geometry::MeshInspector,
  geometry::ExportFaceset>;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& manager)
{
  manager->registerOperations<GeometryOperations>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& manager)
{
  manager->unregisterOperations<GeometryOperations>();
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& manager)
{
  auto& opIcons(manager->operationIcons());
  opIcons.registerOperation<geometry::ImageInspector>(
    [](const std::string& /*unused*/) { return image_inspector_svg; });
  opIcons.registerOperation<geometry::MeshInspector>(
    [](const std::string& /*unused*/) { return mesh_inspector_svg; });
  opIcons.registerOperation<geometry::ExportFaceset>(
    [](const std::string& /*unused*/) { return export_faceset_svg; });
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& manager)
{
  auto& opIcons(manager->operationIcons());
  opIcons.unregisterOperation<geometry::ImageInspector>();
  opIcons.unregisterOperation<geometry::MeshInspector>();
  opIcons.unregisterOperation<geometry::ExportFaceset>();
}
} // namespace operators
} // namespace vtk
} // namespace extension
} // namespace smtk

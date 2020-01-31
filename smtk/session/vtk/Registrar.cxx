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
#include "smtk/session/vtk/Registrar.h"

#include "smtk/session/vtk/operators/Export.h"
#include "smtk/session/vtk/operators/Import.h"
#include "smtk/session/vtk/operators/LegacyRead.h"
#include "smtk/session/vtk/operators/Read.h"
#include "smtk/session/vtk/operators/Write.h"

#include "smtk/session/vtk/Geometry.h"
#include "smtk/session/vtk/Resource.h"

#include "smtk/geometry/Generator.h"

#include "smtk/operation/groups/ExporterGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace session
{
namespace vtk
{

namespace
{
typedef std::tuple<Export, Import, LegacyRead, Read, Write> OperationList;

class RegisterVTKBackend : public smtk::geometry::Supplier<RegisterVTKBackend>
{
public:
  bool valid(const Specification& in) const override
  {
    smtk::extension::vtk::source::Backend backend;
    return std::get<1>(in).index() == backend.index();
  }

  GeometryPtr operator()(const Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<smtk::session::vtk::Resource>(std::get<0>(in));
    if (rsrc)
    {
      auto provider = new Geometry(rsrc);
      return GeometryPtr(provider);
    }
    throw std::invalid_argument("Not a VTK-session resource.");
    return nullptr;
  }
};
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::session::vtk::Resource>(read, write);
  static bool vtkBackend = RegisterVTKBackend::registerClass();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Read>();
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation(smtk::common::typeName<smtk::session::vtk::LegacyRead>(), "exodus");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Write>();

  smtk::operation::ExporterGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Export>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Import>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::vtk::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::ReaderGroup(operationManager).unregisterOperation<smtk::session::vtk::Read>();

  smtk::operation::ReaderGroup(operationManager)
    .unregisterOperation<smtk::session::vtk::LegacyRead>();

  smtk::operation::WriterGroup(operationManager).unregisterOperation<smtk::session::vtk::Write>();

  smtk::operation::ExporterGroup(operationManager)
    .unregisterOperation<smtk::session::vtk::Export>();

  smtk::operation::ImporterGroup(operationManager)
    .unregisterOperation<smtk::session::vtk::Import>();

  operationManager->unregisterOperations<OperationList>();
}
}
}
}

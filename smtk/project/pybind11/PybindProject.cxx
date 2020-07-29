//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <pybind11/pybind11.h>
#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindContainer.h"
#include "PybindManager.h"
#include "PybindMetadata.h"
#include "PybindMetadataContainer.h"
#include "PybindObserver.h"
#include "PybindOperation.h"
#include "PybindOperationFactory.h"
#include "PybindProject.h"
#include "PybindRegistrar.h"
#include "PybindResourceContainer.h"
#include "PybindTags.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(project, m)
{
  m.doc() = "<description of project>";
  py::module std = m.def_submodule("std", "<description>");
  py::module smtk = m.def_submodule("smtk", "<description>");
  py::module project = smtk.def_submodule("project", "<description>");
  py::module detail = project.def_submodule("detail", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_<smtk::project::IdTag> smtk_project_IdTag = pybind11_init_smtk_project_IdTag(project);
  py::class_<smtk::project::IndexTag> smtk_project_IndexTag =
    pybind11_init_smtk_project_IndexTag(project);
  py::class_<smtk::project::LocationTag> smtk_project_LocationTag =
    pybind11_init_smtk_project_LocationTag(project);
  PySharedPtrClass<smtk::project::Manager> smtk_project_Manager =
    pybind11_init_smtk_project_Manager(project);
  py::class_<smtk::project::Metadata> smtk_project_Metadata =
    pybind11_init_smtk_project_Metadata(project);
  py::class_<smtk::project::NameTag> smtk_project_NameTag =
    pybind11_init_smtk_project_NameTag(project);
  py::class_<smtk::project::OperationFactory> smtk_project_OperationFactory =
    pybind11_init_smtk_project_OperationFactory(project);
  py::class_<smtk::project::Registrar> smtk_project_Registrar =
    pybind11_init_smtk_project_Registrar(project);
  py::class_<smtk::project::ResourceContainer> smtk_project_ResourceContainer =
    pybind11_init_smtk_project_ResourceContainer(project);
  py::class_<smtk::project::RoleTag> smtk_project_RoleTag =
    pybind11_init_smtk_project_RoleTag(project);
  pybind11_init_smtk_project_EventType(project);
  pybind11_init_smtk_project_detail_id(detail);
  pybind11_init_smtk_project_detail_index(detail);
  pybind11_init_smtk_project_detail_location(detail);
  pybind11_init_smtk_project_detail_name(detail);
  pybind11_init_smtk_project_detail_role(detail);
  PySharedPtrClass<smtk::project::Operation, smtk::operation::XMLOperation> smtk_project_Operation =
    pybind11_init_smtk_project_Operation(project);
  PySharedPtrClass<smtk::project::Project,
    smtk::resource::DerivedFrom<smtk::project::Project, smtk::resource::Resource> >
    smtk_project_Project = pybind11_init_smtk_project_Project(project);
}

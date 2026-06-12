//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Definition_h
#define pybind_smtk_job_Definition_h

#include "smtk/job/Definition.h"
#include "smtk/job/Queue.h"
#include "smtk/job/State.h"
#include "smtk/job/Status.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::job::Definition> pybind11_init_smtk_job_Definition(py::module &m)
{
  PySharedPtrClass<smtk::job::Definition> instance(m, "Definition");
  instance
    .def_static("create", (std::shared_ptr<smtk::job::Definition> (*)()) &smtk::job::Definition::create)

    .def("name", &smtk::job::Definition::name)
    .def("setName", &smtk::job::Definition::setName, py::arg("name"))

    .def("description", &smtk::job::Definition::description)
    .def("setDescription", &smtk::job::Definition::setDescription, py::arg("description"))

    .def("script", &smtk::job::Definition::script)
    .def("setScript", &smtk::job::Definition::setScript, py::arg("script"))

    .def("containerImage", &smtk::job::Definition::containerImage)
    .def("setContainerImage", &smtk::job::Definition::setContainerImage, py::arg("image_url"))

    .def("caseDirectoryMountPoint", &smtk::job::Definition::caseDirectoryMountPoint)
    .def("setCaseDirectoryMountPoint", &smtk::job::Definition::setCaseDirectoryMountPoint, py::arg("mount_point"))

    .def("appendStage", [](
        smtk::job::Definition& definition,
        const std::string& name,
        const std::string& description,
        const std::filesystem::path& log) -> std::shared_ptr<smtk::job::Stage>
      {
        return definition.appendStage(name, description, log);
      }, py::arg("name"), py::arg("description"), py::arg("log_path")
    )
    .def("appendStage", [](
        smtk::job::Definition& definition,
        const std::string& name,
        const std::string& description) -> std::shared_ptr<smtk::job::Stage>
      {
        return definition.appendStage(name, description);
      }, py::arg("name"), py::arg("description")
    )
    .def("stages", &smtk::job::Definition::stages)

    ;
  return instance;
}

#endif

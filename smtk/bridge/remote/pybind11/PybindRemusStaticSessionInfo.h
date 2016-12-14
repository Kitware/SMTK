//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_remote_RemusStaticSessionInfo_h
#define pybind_smtk_bridge_remote_RemusStaticSessionInfo_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/remote/RemusStaticSessionInfo.h"

namespace py = pybind11;

py::class_< smtk::bridge::remote::RemusStaticSessionInfo > pybind11_init_smtk_bridge_remote_RemusStaticSessionInfo(py::module &m)
{
  py::class_< smtk::bridge::remote::RemusStaticSessionInfo > instance(m, "RemusStaticSessionInfo");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::remote::RemusConnectionPtr, ::remus::proto::JobRequirements const &, ::std::string const &>())
    .def(py::init<::smtk::bridge::remote::RemusStaticSessionInfo const &>())
    .def("__call__", (smtk::model::SessionPtr (smtk::bridge::remote::RemusStaticSessionInfo::*)() const) &smtk::bridge::remote::RemusStaticSessionInfo::operator())
    .def("deepcopy", (smtk::bridge::remote::RemusStaticSessionInfo & (smtk::bridge::remote::RemusStaticSessionInfo::*)(::smtk::bridge::remote::RemusStaticSessionInfo const &)) &smtk::bridge::remote::RemusStaticSessionInfo::operator=)
    .def("staticSetup", &smtk::bridge::remote::RemusStaticSessionInfo::staticSetup, py::arg("optName"), py::arg("optVal"))
    .def("name", &smtk::bridge::remote::RemusStaticSessionInfo::name)
    .def("tags", &smtk::bridge::remote::RemusStaticSessionInfo::tags)
    .def_readwrite("m_conn", &smtk::bridge::remote::RemusStaticSessionInfo::m_conn)
    .def_readwrite("m_meshType", &smtk::bridge::remote::RemusStaticSessionInfo::m_meshType)
    .def_readwrite("m_name", &smtk::bridge::remote::RemusStaticSessionInfo::m_name)
    .def_readwrite("m_tags", &smtk::bridge::remote::RemusStaticSessionInfo::m_tags)
    .def_readwrite("m_operatorXML", &smtk::bridge::remote::RemusStaticSessionInfo::m_operatorXML)
    ;
  return instance;
}

#endif

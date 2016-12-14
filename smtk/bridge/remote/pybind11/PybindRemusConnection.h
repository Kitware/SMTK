//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_remote_RemusConnection_h
#define pybind_smtk_bridge_remote_RemusConnection_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/remote/RemusConnection.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::remote::RemusConnection > pybind11_init_smtk_bridge_remote_RemusConnection(py::module &m)
{
  PySharedPtrClass< smtk::bridge::remote::RemusConnection > instance(m, "RemusConnection");
  instance
    .def(py::init<::smtk::bridge::remote::RemusConnection const &>())
    .def("deepcopy", (smtk::bridge::remote::RemusConnection & (smtk::bridge::remote::RemusConnection::*)(::smtk::bridge::remote::RemusConnection const &)) &smtk::bridge::remote::RemusConnection::operator=)
    .def_property("modelManager", &smtk::bridge::remote::RemusConnection::modelManager, &smtk::bridge::remote::RemusConnection::setModelManager)
    .def("classname", &smtk::bridge::remote::RemusConnection::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::RemusConnection> (*)()) &smtk::bridge::remote::RemusConnection::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::RemusConnection> (*)(::std::shared_ptr<smtk::bridge::remote::RemusConnection> &)) &smtk::bridge::remote::RemusConnection::create, py::arg("ref"))
    .def("addSearchDir", &smtk::bridge::remote::RemusConnection::addSearchDir, py::arg("searchDir"))
    .def("clearSearchDirs", &smtk::bridge::remote::RemusConnection::clearSearchDirs, py::arg("clearDefaultsToo") = false)
    .def("connectToServer", &smtk::bridge::remote::RemusConnection::connectToServer, py::arg("hostname") = "local", py::arg("port") = remus::server::CLIENT_PORT)
    .def("sessionTypeNames", &smtk::bridge::remote::RemusConnection::sessionTypeNames)
    .def("staticSetup", &smtk::bridge::remote::RemusConnection::staticSetup, py::arg("sessionName"), py::arg("optName"), py::arg("optVal"))
    .def("beginSession", &smtk::bridge::remote::RemusConnection::beginSession, py::arg("sessionName"))
    .def("endSession", &smtk::bridge::remote::RemusConnection::endSession, py::arg("sessionId"))
    .def("findSession", &smtk::bridge::remote::RemusConnection::findSession, py::arg("sessionId"))
    .def("supportedFileTypes", &smtk::bridge::remote::RemusConnection::supportedFileTypes, py::arg("sessionName") = std::string())
    .def("readFile", &smtk::bridge::remote::RemusConnection::readFile, py::arg("fileName"), py::arg("fileType") = std::string(), py::arg("sessionName") = std::string())
    .def("operatorNames", (std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > (smtk::bridge::remote::RemusConnection::*)(::std::string const &)) &smtk::bridge::remote::RemusConnection::operatorNames, py::arg("sessionName"))
    .def("operatorNames", (std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > (smtk::bridge::remote::RemusConnection::*)(::smtk::common::UUID const &)) &smtk::bridge::remote::RemusConnection::operatorNames, py::arg("sessionId"))
    .def("createOperator", (smtk::model::OperatorPtr (smtk::bridge::remote::RemusConnection::*)(::smtk::common::UUID const &, ::std::string const &)) &smtk::bridge::remote::RemusConnection::createOperator, py::arg("sessionOrModel"), py::arg("opName"))
    .def("createOperator", (smtk::model::OperatorPtr (smtk::bridge::remote::RemusConnection::*)(::std::string const &, ::std::string const &)) &smtk::bridge::remote::RemusConnection::createOperator, py::arg("sessionName"), py::arg("opName"))
    .def("fetchWholeModel", &smtk::bridge::remote::RemusConnection::fetchWholeModel, py::arg("modelId"))
    .def("jsonRPCRequest", (cJSON * (smtk::bridge::remote::RemusConnection::*)(::cJSON *, ::remus::proto::JobRequirements const &)) &smtk::bridge::remote::RemusConnection::jsonRPCRequest, py::arg("req"), py::arg("jreq"))
    .def("jsonRPCRequest", (cJSON * (smtk::bridge::remote::RemusConnection::*)(::std::string const &, ::remus::proto::JobRequirements const &)) &smtk::bridge::remote::RemusConnection::jsonRPCRequest, py::arg("req"), py::arg("jreq"))
    .def("jsonRPCNotification", (void (smtk::bridge::remote::RemusConnection::*)(::cJSON *, ::remus::proto::JobRequirements const &)) &smtk::bridge::remote::RemusConnection::jsonRPCNotification, py::arg("req"), py::arg("jreq"))
    .def("jsonRPCNotification", (void (smtk::bridge::remote::RemusConnection::*)(::std::string const &, ::remus::proto::JobRequirements const &)) &smtk::bridge::remote::RemusConnection::jsonRPCNotification, py::arg("req"), py::arg("jreq"))
    .def("connection", &smtk::bridge::remote::RemusConnection::connection)
    .def("log", &smtk::bridge::remote::RemusConnection::log)
    ;
  return instance;
}

#endif

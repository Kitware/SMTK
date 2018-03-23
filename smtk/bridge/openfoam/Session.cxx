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
#include "smtk/bridge/openfoam/Session.h"

#include "smtk/common/PythonInterpreter.h"
#include "smtk/common/UUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/ArrangementHelper.h"
#include "smtk/model/Model.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>

#include <boost/cstdint.hpp>
//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <sstream>

using namespace smtk::model;
using namespace smtk::common;

namespace
{
std::string scratch_dir = SMTK_SCRATCH_DIR;
}

namespace smtk
{
namespace bridge
{
namespace openfoam
{

Session::Session()
{
  const char* disclaimer =
    "This session is a prototype only. It will allow you to:\n"
    "\n"
    "  a) set the working directory\n"
    "  b) set OpenFOAM's main controls\n"
    "  c) create a wind tunnel\n"
    "  d) add an obstacle (from auxiliary geometry) to a wind tunnel\n"
    "  e) annotate the model (boundary conditions, physical properties, etc.)\n"
    "\n"
    "These operations are expected to be performed in this order. If you are\n"
    "running natively with an Ubuntu build (see https://openfoam.org/download/),\n"
    "make sure you source OpenFOAM's environment prior to executing\n"
    "ModelBuilder ('source /opt/openfoam5/etc/bashrc'). If you are running on\n"
    "OS X with an OpenFOAM Docker container, make sure the OpenFOAM image is\n"
    "mounted prior to executing any operations\n"
    "('openfoam-macos-file-system mount'). The exposed operations are designed\n"
    "to demonstrate the ability to integrate OpenFOAM into SMTK. It has been\n"
    "tested using the default values and OpenFOAM's provided 'motorBike.obj'\n"
    "obstacle.\n";

  smtkWarningMacro(this->log(), disclaimer);

  this->initializeOperatorCollection(Session::s_operators);

  this->setWorkingDirectory(
    (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());
}

Session::~Session()
{
}

void Session::createWorkingDirectory() const
{
  std::stringstream s;
  s << "Working directory created at \"" << this->workingDirectory() << "\".";
  smtkInfoMacro(const_cast<Session*>(this)->log(), s.str());
  boost::filesystem::create_directory(boost::filesystem::path(this->m_workingDirectory));
#ifdef __APPLE__
  // Set up the script needed to execute OpenFOAM commands
  std::ofstream file(this->m_workingDirectory + "/run_openfoam.sh");
  file << "#!/bin/bash\n\n";
  file << "OPENFOAM_SCRATCH_DIR=\"$( pwd )\"\n\n";
  file << "cd $OPENFOAM_SCRATCH_DIR\n\n";
  file << "echo \"$@\" > .bashrc\n";
  file << "echo \"exit\" >> .bashrc\n\n";
  file << "openfoam5-macos -p\n";
  file << "rm .bashrc\n";
  file.close();
#endif
}

void Session::removeWorkingDirectory() const
{
  boost::filesystem::remove_all(boost::filesystem::path(this->m_workingDirectory));
}

bool Session::workingDirectoryExists() const
{
  return boost::filesystem::exists(boost::filesystem::path(this->m_workingDirectory));
}

} // namespace openfoam
} // namespace bridge
} // namespace smtk

#include "smtk/bridge/openfoam/Session_json.h"

smtkImplementsModelingKernel(SMTKOPENFOAMSESSION_EXPORT, openfoam, Session_json,
  SessionHasNoStaticSetup, smtk::bridge::openfoam::Session, true /* inherit "universal" operators */
  );

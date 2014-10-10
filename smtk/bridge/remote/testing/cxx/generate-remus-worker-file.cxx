//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/remote/RemusRemoteBridge.h"

#include "smtk/Options.h"

// ++ UserGuide/Model/1 ++
#include "smtk/AutoInit.h"

#ifndef WIN32
#  include <sys/param.h> // Necessary on Mac OS X
#endif // WIN32
#include <stdlib.h> // for realpath/_fullpath

#ifdef SMTK_USE_CGM
// If CGM is included in the build, ensure that it is loaded
smtkComponentInitMacro(smtk_cgm_bridge);
smtkComponentInitMacro(smtk_cgm_read_operator);
smtkComponentInitMacro(smtk_remote_bridge);
#endif // SMTK_USE_CGM
// -- UserGuide/Model/1 --

using namespace smtk::model;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  std::string self = argv[0];
  std::size_t spos = self.find("generate-remus-worker-file");
  if (spos == self.npos)
    {
    std::cerr << "Could not find directory containing this executable.\n";
    return 1;
    }
  std::string binDir = self.substr(0, spos);
  std::string worker = binDir + "smtk-model-server";
  char* realWorkerChar =
#ifndef WIN32
    realpath(worker.c_str(), NULL);
#else // WIN32
    _fullpath(NULL, worker.c_str());
#endif // WIN32
  if (!realWorkerChar)
    {
    std::cerr << "Could not determine location of smtk-model-server.\n";
    return 1;
    }
  std::string realWorker(realWorkerChar);
  free(realWorkerChar);

  StringList typeNames = smtk::bridge::remote::RemusRemoteBridge::availableTypeNames();
  for (StringList::const_iterator it = typeNames.begin(); it != typeNames.end(); ++it)
    {
    std::cout
      << "{\n"
      << "  \"InputType\":\"smtk\",\n"
      << "  \"OutputType\":\"" << *it << "\",\n"
      << "  \"ExecutableName\":\"" << realWorker << "\"\n"
      << "}\n"
      ;
    }
  return 0;
}

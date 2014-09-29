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

#ifdef SMTK_USE_CGM
// If CGM is included in the build, ensure that it is loaded
smtkComponentInitMacro(smtk_cgm_bridge);
#endif // SMTK_USE_CGM
// -- UserGuide/Model/1 --

using namespace smtk::model;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  StringList typeNames = smtk::bridge::remote::RemusRemoteBridge::availableTypeNames();
  for (StringList::const_iterator it = typeNames.begin(); it != typeNames.end(); ++it)
    {
    std::cout
      << "{\n"
      << "  \"InputType\":\"smtk\",\n"
      << "  \"OutputType\":\"" << *it << "\",\n"
      << "  \"ExecutableName\":\"../../../bin/smtk-remote-worker\",\n"
      << "}\n"
      ;
    }
  return 0;
}

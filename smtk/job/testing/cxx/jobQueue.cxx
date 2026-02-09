//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/job/Resource.h"

int jobQueue(int /*unused*/, char* /*unused*/[])
{
  auto resource = smtk::job::Resource::instance();
  if (!resource)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No job resource.");
    return 1;
  }
  return 0;
}

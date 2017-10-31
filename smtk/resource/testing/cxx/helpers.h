//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_testing_cxx_helpers_h
#define smtk_resource_testing_cxx_helpers_h

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{
namespace testing
{

ResourceArray loadTestResources(ManagerPtr resourceManager, int argc, char* argv[]);
}
}
}

#endif

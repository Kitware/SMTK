//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_Wrapper_h
#define smtk_resource_Wrapper_h

#include "smtk/resource/Resource.h"
#include "smtk/resource/Set.h"

namespace smtk
{
namespace resource
{

// Simple container for single Resource plus meta data
struct SMTKCORE_EXPORT Wrapper
{
  ResourcePtr resource;
  Set::Role role;
  Set::State state;
  std::string id;
  std::string link;
};

} // namespace resource
} // namespace smtk

#endif // smtk_resource_Wrapper_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_ResourceWrapper_h
#define smtk_common_ResourceWrapper_h

#include "smtk/common/Resource.h"
#include "smtk/common/ResourceSet.h"

namespace smtk
{
namespace common
{

// Simple container for single Resource plus meta data
struct SMTKCORE_EXPORT ResourceWrapper
{
  ResourcePtr resource;
  Resource::Type type;
  ResourceSet::ResourceRole role;
  ResourceSet::ResourceState state;
  std::string id;
  std::string link;
};

} // namespace common
} // namespace smtk

#endif // smtk_common_ResourceWrapper_h

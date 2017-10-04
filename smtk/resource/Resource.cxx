//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.cxx - Abstract base class for CMB resources
// .SECTION Description
// .SECTION See Also

#include "smtk/resource/Resource.h"

#include "smtk/common/UUIDGenerator.h"

namespace smtk
{
namespace resource
{

Resource::Resource(const common::UUID& myID, Manager* manager)
  : m_id(myID)
  , m_manager(manager)
{
}

Resource::Resource(Manager* manager)
  : m_manager(manager)
{
  this->m_id = smtk::common::UUIDGenerator::instance().random();
}

Resource::~Resource()
{
}

std::string Resource::type2String(Resource::Type t)
{
  switch (t)
  {
    case ATTRIBUTE:
      return "attribute";
    case MODEL:
      return "model";
    case MESH:
      return "mesh";
    default:
      return "";
  }
  return "Error!";
}

Resource::Type Resource::string2Type(const std::string& s)
{
  if (s == "attribute")
  {
    return ATTRIBUTE;
  }
  if (s == "model")
  {
    return MODEL;
  }
  if (s == "mesh")
  {
    return MESH;
  }
  return NUMBER_OF_TYPES;
}

} // namespace resource
} // namespace smtk

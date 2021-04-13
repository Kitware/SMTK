//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_LinkInformation_h
#define smtk_resource_LinkInformation_h

#include "smtk/CoreExports.h"

#include "smtk/common/UUID.h"

namespace smtk
{
namespace resource
{
/// Given a Key, Information about the object to which a link connects can be
/// returned.
struct SMTKCORE_EXPORT LinkInformation
{
  /// A link is either Valid (can be accessed), Invalid (the object to which it
  /// links has been removed), or Unknown (the object is or belongs to a
  /// resource that is not in memory).
  enum class Status : char
  {
    Valid,
    Invalid,
    Unknown
  };
  Status status;

  /// A link connects to either a component or a resource.
  enum class Type : char
  {
    Resource,
    Component
  };
  Type type;

  /// The on-disk location of the linked object
  std::string location;

  /// The id of the linked object
  smtk::common::UUID id;

  /// The role of the link
  Links::RoleType role;
};
} // namespace resource
} // namespace smtk

#endif // smtk_resource_LinkInformation_h

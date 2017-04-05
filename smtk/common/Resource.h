//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.h - Abstract base class for SMTK resources
// .SECTION Description
//   An SMTK resource is one of: attribute manager, model, mesh
// .SECTION See Also

#ifndef smtk_common_Resource_h
#define smtk_common_Resource_h

#include "smtk/CoreExports.h"
#include <string>

namespace smtk {
  namespace common {

class SMTKCORE_EXPORT Resource
{
public:
  virtual ~Resource();

  /// Identifies resource type
  enum Type
    {
    ATTRIBUTE = 0,
    MODEL,
    MESH,         // future
    NUMBER_OF_TYPES
    };

  virtual Resource::Type resourceType() const = 0;

  static std::string type2String(Resource::Type t);
  static Resource::Type string2Type(const std::string &s);

protected:
  Resource();
};

  }
}

#endif  // smtk_common_Resource_h

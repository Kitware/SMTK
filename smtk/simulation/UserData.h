//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkAttribute.h - Represents a standalone piece of simulation information
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_simulation_UserData_h
#define __smtk_simulation_UserData_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace simulation
{
//derive from this class to create custom user data.
class SMTKCORE_EXPORT UserData
{
public:
  static smtk::simulation::UserDataPtr New()
  {
    return smtk::simulation::UserDataPtr(new UserData());
  }

  virtual ~UserData();

protected:
  UserData();
};

} // namespace simulation
} // namespace smtk

#endif // __smtk_simulation_UserData_h

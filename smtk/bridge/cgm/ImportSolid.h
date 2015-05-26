//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_ImportSolid_h
#define __smtk_session_cgm_ImportSolid_h

#include "smtk/bridge/cgm/Exports.h" // for SMTKCGMSESSION_EXPORT
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/common/UUID.h"

namespace smtk {
  namespace model {
    class Manager;
  }
}

namespace smtk {
  namespace bridge {
    namespace cgm {

/**\brief Load a solid model using CGM.
  *
  */
class SMTKCGMSESSION_EXPORT ImportSolid
{
public:
  static smtk::common::UUIDArray fromFilenameIntoManager(
    const std::string& filename,
    const std::string& filetype,
    smtk::model::ManagerPtr manager);
};

    } // namespace cgm
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_ImportSolid_h

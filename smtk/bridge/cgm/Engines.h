//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_cgm_Engines_h
#define __smtk_bridge_cgm_Engines_h

#include "smtk/Options.h" // for CGM_HAVE_VERSION_H
#include "smtk/bridge/cgm/cgmSMTKExports.h"
#ifdef CGM_HAVE_VERSION_H
#  include "cgm_version.h"
#endif

#include <string>
#include <vector>

namespace smtk {
  namespace bridge {
    namespace cgm {

/**\brief Ensure that CGMA has been initialized.
  *
  * This registers attributes with CGM and prepares one
  * GeometryQueryEngine to be the default.
  * You may change the default engine later.
  */
class CGMSMTK_EXPORT Engines
{
public:
  static bool areInitialized();

  static bool isInitialized(
    const std::string& engine,
    const std::vector<std::string>& args = std::vector<std::string>());
  static bool setDefault(const std::string& engine);

  static std::vector<std::string> listEngines();

  static bool shutdown();
};

    } // namespace cgm
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_cgm_Engines_h

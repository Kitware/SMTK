//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_Engines_h
#define __smtk_session_cgm_Engines_h

#include "smtk/Options.h" // for CGM_HAVE_VERSION_H
#include "smtk/session/cgm/Exports.h"
#ifdef CGM_HAVE_VERSION_H
#include "cgm_version.h"
#endif

#include <string>
#include <vector>

class Body;

namespace smtk
{
namespace session
{
namespace cgm
{

/**\brief Ensure that CGMA has been initialized.
  *
  * This registers attributes with CGM and prepares one
  * GeometryQueryEngine to be the default.
  * You may change the default engine later.
  */
class SMTKCGMSESSION_EXPORT Engines
{
public:
  static bool areInitialized();

  static bool isInitialized(
    const std::string& engine, const std::vector<std::string>& args = std::vector<std::string>());
  static bool setDefault(const std::string& engine);
  static std::string currentEngine();

  static std::vector<std::string> listEngines();

  static bool shutdown();
};

} // namespace cgm
} // namespace session
} // namespace smtk

#endif // __smtk_session_cgm_Engines_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_BridgeRegistrar_h
#define __smtk_model_BridgeRegistrar_h
#ifndef SHIBOKEN_SKIP

#include "smtk/PublicPointerDefs.h"
#include "smtk/Function.h"
#include "smtk/model/StringData.h"

#include <map>

namespace smtk {
  namespace model {

/**\brief A (generic) function-pointer to perform pre-construction bridge setup.
  *
  * When a bridge is registered (by calling BridgeRegistrar::registerBridge),
  * a function of this signature may be passed to it, indicating that this
  * function may be called before a bridge is constructed.
  *
  * The method may be called multiple times before a bridge instance is
  * constructed with a cumulative effect if the setup option names vary.
  *
  * If the function returns a positive number, then the setup is accepted as
  * valid and construction may proceed (or setup called again).
  * If it returns 0, then the setup was ignored but construction may proceed.
  * If it returns a negative number then construction should not proceed.
  * The latter can happen for a variety of reasons, including a license limit
  * being reached; an invalid modeling kernel specified; or an impossible
  * configuration.
  *
  * The effect of setup option names is entirely dependent on the bridge
  * and thus methods to recover from an impossible configuration are left
  * to Bridge subclasses to implement.
  */
typedef smtk::function<
  int (const std::string&, const StringList& val)> BridgeStaticSetup;

/// A (generic) function-pointer to construct a bridge instance.
typedef smtk::function<BridgePtr()> BridgeConstructor;

/// A record associating bridge information with a constructor method.
struct StaticBridgeInfo {
  std::string Name;
  BridgeStaticSetup Setup;
  BridgeConstructor Constructor;
  std::string Tags;
  bool TagsParsed;
  std::string Site;
  StringList Engines;
  StringData FileTypes;

  StaticBridgeInfo() : TagsParsed(false) { }
  StaticBridgeInfo(
    const std::string& bname,
    const std::string& btags,
    BridgeStaticSetup bsetup,
    BridgeConstructor bctor)
    : Name(bname), Setup(bsetup), Constructor(bctor), Tags(btags), TagsParsed(false)
    { }
};

/// A map of bridge names to constructors.
typedef std::map<std::string,StaticBridgeInfo> BridgeConstructors;

/**\brief A helper for bridges that do not perform static setup.
  *
  * If a bridge subclass does not require setup,
  * pass this to BridgeRegistrar::registerBridge().
  */
inline int BridgeHasNoStaticSetup(
  const std::string&,
  const StringList&)
{
  return 1;
}

/**\brief A static class for holding information about bridges to modeling kernels.
  *
  * This class is not wrapped for use in Python because shiboken
  * cannot parse headers for boost::bind and boost::function.
  * Use the model manager to identify available bridges and create sessions
  * as BRepModel (a subclass of Manager) exposes methods that can be wrapped.
  */
class SMTKCORE_EXPORT BridgeRegistrar
{
public:
  static bool registerBridge(
    const std::string& bname,
    const std::string& tags,
    BridgeStaticSetup bsetup,
    BridgeConstructor bctor);
  static StringList bridgeNames();
  static std::string bridgeTags(const std::string& bname);
  static std::string bridgeSite(const std::string& bname);
  static StringList bridgeEngines(const std::string& bname);
  static StringList bridgeFileTypes(
    const std::string& bname, const std::string& engine = std::string());
  static BridgeStaticSetup bridgeStaticSetup(const std::string& bname);
  static BridgeConstructor bridgeConstructor(const std::string& bname);
  static BridgePtr createBridge(const std::string& bname);

protected:
  static void cleanupBridges();
  static void parseTags(StaticBridgeInfo& bridge);

  static BridgeConstructors* s_bridges;
};

  } // namespace model
} // namespace smtk

#endif // SHIBOKEN_SKIP
#endif // __smtk_model_BridgeRegistrar_h

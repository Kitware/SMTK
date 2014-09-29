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

/// A (generic) function-pointer to construct a bridge instance.
typedef smtk::function<BridgePtr()> BridgeConstructor;

/// A record associating bridge information with a constructor method.
struct StaticBridgeInfo {
  std::string Name;
  BridgeConstructor Constructor;
  StringList Tags;
  StringList FileTypes;

  StaticBridgeInfo() { }
  StaticBridgeInfo(
    const std::string& bname,
    BridgeConstructor bctor,
    const StringList& btags = StringList(),
    const StringList& btype = StringList())
    : Name(bname), Constructor(bctor), Tags(btags), FileTypes(btype)
    { }
};

/// A map of bridge names to constructors.
typedef std::map<std::string,StaticBridgeInfo> BridgeConstructors;

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
    const StringList& fileTypes,
    const StringList& tags,
    BridgeConstructor bctor);
  static StringList bridgeNames();
  static StringList bridgeFileTypes(const std::string& bname);
  static StringList bridgeTags(const std::string& bname);
  static BridgeConstructor bridgeConstructor(const std::string& bname);
  static BridgePtr createBridge(const std::string& bname);

protected:
  static void cleanupBridges();

  static BridgeConstructors* s_bridges;
};

  } // namespace model
} // namespace smtk

#endif // SHIBOKEN_SKIP
#endif // __smtk_model_BridgeRegistrar_h

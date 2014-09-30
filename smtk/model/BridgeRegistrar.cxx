//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/BridgeRegistrar.h"

#include "smtk/model/Bridge.h"

#include <stdlib.h> // for atexit()

using namespace smtk::common;

namespace smtk {
  namespace model {

/// A map of all the bridges which may be instantiated (indexed by name).
BridgeConstructors* BridgeRegistrar::s_bridges = NULL;

void BridgeRegistrar::cleanupBridges()
{
  delete BridgeRegistrar::s_bridges;
}

bool BridgeRegistrar::registerBridge(const std::string& bname, const StringList& fileTypes, BridgeConstructor bctor)
{
  if (!BridgeRegistrar::s_bridges)
    {
    BridgeRegistrar::s_bridges = new BridgeConstructors;
    atexit(cleanupBridges);
    }
  if (!bname.empty() && bctor)
    {
    StaticBridgeInfo entry(fileTypes, bctor);
    (*BridgeRegistrar::s_bridges)[bname] = entry;
    return true;
    }
  else if (!bname.empty())
    { // unregister the bridge of the given name.
    BridgeRegistrar::s_bridges->erase(bname);
    // FIXME: We should ensure that no registered Bridge sessions are of type bname.
    //        Presumably, by deleting all such sessions and removing their entities
    //        from storage.
    }
  return false;
}

/// Return a list of the names of each bridge subclass whose constructor has been registered with SMTK.
StringList BridgeRegistrar::bridgeNames()
{
  StringList result;
  for (BridgeConstructors::const_iterator it = s_bridges->begin(); it != s_bridges->end(); ++it)
    result.push_back(it->first);
  return result;
}

/// Return the list of file types this bridge can read (currently: a list of file extensions).
StringList BridgeRegistrar::bridgeFileTypes(const std::string& bname)
{
  BridgeConstructors::const_iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    return it->second.first;
  StringList result;
  return result;
}

/**\brief Return a function to construct a Bridge instance given its class-specific name. Or NULL if you pass an invalid name.
  *
  * After calling the constructor, you most probably want to call
  * registerBridgeSession with the new bridge's UUID.
  */
BridgeConstructor BridgeRegistrar::bridgeConstructor(const std::string& bname)
{
  BridgeConstructor result = NULL;
  BridgeConstructors::const_iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    result = it->second.second;
  return result;
}

/**\brief Return a new Bridge instance given its class-specific name. Or NULL if you pass an invalid name.
  *
  */
BridgePtr BridgeRegistrar::createBridge(const std::string& bname)
{
  BridgeConstructor ctor = NULL;
  BridgeConstructors::const_iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    ctor = it->second.second;
  if (ctor)
    return ctor();
  return BridgePtr();
}

  } // model namespace
} // smtk namespace

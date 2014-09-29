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

/// Called to delete allocated memory as the program exits.
void BridgeRegistrar::cleanupBridges()
{
  delete BridgeRegistrar::s_bridges;
}

/**\brief Register a bridge "type" for use by an application.
  *
  * The type (\a bname) must be unique.
  * For most bridges, it will be the name of the modeling kernel
  * that is bridged to SMTK (e.g., discrete).
  * However, for
  * + bridges that present multiple modeling kernels (e.g., "cgm"),
  *   it should uniquely include those kernels (e.g., "cgm{Cubit,OCC}");
  * + remote bridges, it should be unique across any connection
  *   configuration information (i.e., the name of the server and
  *   remote endpoint of the remote bridge, the modeling kernel used by
  *   the remote bridge, etc.).
  *
  * This allows applications to query available bridges by
  * the types of files they can read, the name of the bridge
  * class, or by arbitrary tags associated with them.
  * Once a query identifies a suitable bridge, the application
  * can construct bridges of the the given type using the
  * constructor or by calling convenience routines such as
  * createBridge and createBridgeWithTags.
  *
  * Again, note that the bridge name \a bname must be unique;
  * only one constructor is stored per \a bname.
  * Remus and other remote bridges should include
  * disambiguating information in both \a bname and \a tags.
  * If a tag begins with "{" and ends with "}" then
  * applications may assume it is a JSON dictionary and
  * use values in the dictionary for presentation to users.
  * The latter is much more friendly to users than the former.
  * For example, a tag might be<pre>
  * {"class":"remus",
  *  "server":"tcp://localhost",
  *  "worker-host":"foo.kitware.com",
  *  "bridge":"cgm", "engine":"OCC",
  *  "filesys":"8dfcb75c-a3b5-4e10-a9dd-85623afe3372"}
  *  </pre>
  * for the \a bname
  * "smtk::model[cgm{OCC}] foo.kitware.com@tcp://localhost".
  *
  * The tag names in the example above should be present for
  * remus remote bridges, with "engine" being optional depending
  * on whether the bridge supports multiple modeling kernels.
  *
  * Because constructors are smtk::function instances,
  * setup can be included as part of the construction
  * process. For instance, the RemusRemoteBridge class
  * calls registerBridge multiple times with different
  * names and bridge types whenever a connection to a
  * new Remus server is opened: one registerBridge call
  * for each remote bridge type. The constructor method
  * passed creates a RemusRemoteBridge instance and
  * configures it to (1) use a given Remus server connection
  * and (2) prepare an actual bridge of the proper type
  * on the SMTK model worker at the far end of the server
  * connection.
  */
bool BridgeRegistrar::registerBridge(
  const std::string& bname,
  const StringList& fileTypes,
  const StringList& tags,
  BridgeConstructor bctor)
{
  if (!BridgeRegistrar::s_bridges)
    {
    BridgeRegistrar::s_bridges = new BridgeConstructors;
    atexit(cleanupBridges);
    }
  if (!bname.empty() && bctor)
    {
    StaticBridgeInfo entry(bname, bctor, tags, fileTypes);
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
    return it->second.FileTypes;
  StringList result;
  return result;
}

/**\brief Return the list of tag strings that describe this bridge.
  *
  * Note that any entry beginning with "{" and ending with "}" should
  * be assumed to be a JSON dictionary containing fields to present
  * to a user when selecting bridges.
  * The JSON may also be used to group bridges by capabilities.
  */
StringList BridgeRegistrar::bridgeTags(const std::string& bname)
{
  BridgeConstructors::const_iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    return it->second.Tags;
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
    result = it->second.Constructor;
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
    ctor = it->second.Constructor;
  if (ctor)
    return ctor();
  return BridgePtr();
}

  } // model namespace
} // smtk namespace

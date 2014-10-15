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

#include "smtk/io/ImportJSON.h"

#include "cJSON.h"

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

/**\brief Parse the JSON tag data for the given \a bridge.
  *
  * Look for specific keys in bridge.Tag and use them
  * to populate other bridge member variables.
  */
void BridgeRegistrar::parseTags(StaticBridgeInfo& bridge)
{
  bridge.TagsParsed = true;
  if (bridge.Tags.empty())
    return;

  cJSON* json = cJSON_Parse(bridge.Tags.c_str());
  if (!json)
    return;

  if (json->type == cJSON_Object)
    {
    cJSON* kernel = cJSON_GetObjectItem(json, "kernel");
    if (
      kernel &&
      kernel->type == cJSON_String &&
      kernel->valuestring &&
      kernel->valuestring[0])
      bridge.Name = kernel->valuestring;

    cJSON* engines = cJSON_GetObjectItem(json, "engines");
    if (
      engines &&
      engines->type == cJSON_Array)
      {
      bridge.Engines.clear();
      bridge.FileTypes.clear();
      for (cJSON* engine = engines->child; engine; engine = engine->next)
        {
        if (engine->type == cJSON_Object)
          {
          std::string curEngine;
          cJSON* einfo;
          einfo = cJSON_GetObjectItem(engine, "name");
          if (
            einfo &&
            einfo->type == cJSON_String &&
            einfo->valuestring &&
            einfo->valuestring[0])
            {
            curEngine = einfo->valuestring;
            bridge.Engines.push_back(curEngine);
            }
          einfo = cJSON_GetObjectItem(engine, "filetypes");
          if (
            einfo &&
            einfo->type == cJSON_Array &&
            einfo->child)
            {
            StringList fileTypes;
            smtk::io::ImportJSON::getStringArrayFromJSON(einfo->child, fileTypes);
            if (curEngine.empty())
              curEngine = "*";
            bridge.FileTypes[curEngine] = fileTypes;
            }
          }
        }
      }
    }

  cJSON_Delete(json);
}

/**\brief Register a bridge "type" for use by an application.
  *
  * The type (\a bname) must be unique.
  * To make bridge names unique the following naming scheme is used:
  * "smtk::model[kernel{options},site]@server:port".
  * The leading "smtk::model" appears so that Remus can distinguish
  * between models and meshes. The model is then qualified by an
  * expression in square brackets which indicates the modeling
  * kernel at the remote end (including any configuration options
  * for that kernel in curly brackets) and the site name.
  * The site name may be a hostname or, where a filesystem is shared
  * by multiple hosts, a name identifying the shared filesystem.
  * Finally, the Remus server and port number appear at the end of
  * the name so that multiple engines with the same configuration
  * may be distinguished by the Remus server they registered with.
  *
  * This class allows applications to query available bridges by
  * the types of files they can read, the name of the bridge
  * class, or by arbitrary tags associated with them identifying
  * configuration options.
  * Once a query identifies a suitable bridge, the application
  * can construct bridges of the the given type using the
  * constructor or by calling convenience routines such as
  * createBridge and createBridgeWithTags.
  *
  * Again, note that the bridge name \a bname must be unique;
  * only one constructor is stored per \a bname.
  * Remus and other remote bridges should include
  * disambiguating information in both \a bname and \a tags.
  * The tag is assumed to be a JSON string.
  * See the SMTK User's Guide for more information
  * on its structure.
  * An example tag might be<pre>
  * {"class":"remus",
  *  "server":"tcp://localhost",
  *  "site":"foo.kitware.com",
  *  "bridge":"cgm", "engine":"OCC",
  *  "filetypes":[".brep", ".occ", ".stl"],
  *  "filesys":"8dfcb75c-a3b5-4e10-a9dd-85623afe3372"}
  *  </pre>
  * for the \a bname
  * "smtk::model[cgm{OCC},foo.kitware.com]@tcp://localhost".
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
  const std::string& btags,
  BridgeConstructor bctor)
{
  if (!BridgeRegistrar::s_bridges)
    {
    BridgeRegistrar::s_bridges = new BridgeConstructors;
    atexit(cleanupBridges);
    }
  if (!bname.empty() && bctor)
    {
    StaticBridgeInfo entry(bname, btags, bctor);
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
StringList BridgeRegistrar::bridgeFileTypes(
  const std::string& bname,
  const std::string& bengine)
{
  BridgeConstructors::iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    {
    if (!it->second.TagsParsed)
      BridgeRegistrar::parseTags(it->second);
    StringData::const_iterator eit;
    if (bengine.empty() && !it->second.FileTypes.empty())
      return it->second.FileTypes.begin()->second;
    else if (
      !bengine.empty() &&
      (eit = it->second.FileTypes.find(bengine)) != it->second.FileTypes.end())
      return eit->second;
    }
  StringList empty;
  return empty;
}

/**\brief Return the tag string that describes this bridge.
  *
  * The tag should be a JSON dictionary containing fields to present
  * to a user when selecting bridges.
  * The JSON may also be used to group bridges by capabilities.
  */
std::string BridgeRegistrar::bridgeTags(const std::string& bname)
{
  BridgeConstructors::const_iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    return it->second.Tags;
  std::string empty;
  return empty;
}

/**\brief Return the site that describes the filesystem for this bridge.
  *
  * The site should be a unique, user-presentable label identifying
  * the filesystem available to the model worker.
  * If empty, the site should be interpreted to be the root
  * filesystem of the local machine.
  */
std::string BridgeRegistrar::bridgeSite(const std::string& bname)
{
  BridgeConstructors::iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    {
    if (!it->second.TagsParsed)
      BridgeRegistrar::parseTags(it->second);
    return it->second.Site;
    }
  std::string empty;
  return empty;
}

/**\brief Return the list of engines that the bridge provides.
  *
  * Some bridges, such as CGM, expose multiple solid modeling kernels;
  * in this case, we call the underlying kernels "engines" and
  * provide a way to obtain the list of engines for a given bridge name.
  */
StringList BridgeRegistrar::bridgeEngines(const std::string& bname)
{
  BridgeConstructors::iterator it = s_bridges->find(bname);
  if (it != s_bridges->end())
    {
    if (!it->second.TagsParsed)
      BridgeRegistrar::parseTags(it->second);
    return it->second.Engines;
    }
  StringList empty;
  return empty;
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

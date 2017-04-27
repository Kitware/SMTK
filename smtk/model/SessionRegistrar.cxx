//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SessionRegistrar.h"

#include "smtk/model/Session.h"

#include "smtk/io/LoadJSON.h"

#include "cJSON.h"

#include <stdlib.h> // for atexit()

using namespace smtk::common;

namespace smtk
{
namespace model
{

/// A map of all the sessions which may be instantiated (indexed by name).
SessionConstructors* SessionRegistrar::s_sessions(bool del)
{
  static SessionConstructors* sessions = SMTK_FUNCTION_INIT;
  if (del)
  {
    if (sessions)
    {
      //std::cout << "Deleting session list " << sessions << " (" << &sessions << ")" << "\n";
      delete sessions;
      sessions = SMTK_FUNCTION_INIT;
    }
  }
  else
  {
    if (!sessions)
    {
      sessions = new SessionConstructors();
      atexit(cleanupSessions);
      //std::cout << "Allocating session list " << sessions << " (" << &sessions << ")" << "\n";
    }
  }
  return sessions;
}

/// Called to delete allocated memory as the program exits.
void SessionRegistrar::cleanupSessions()
{
  s_sessions(true);
}

/**\brief Parse the JSON tag data for the given \a session.
  *
  * Look for specific keys in session.Tag and use them
  * to populate other session member variables.
  */
void SessionRegistrar::parseTags(StaticSessionInfo& session)
{
  session.TagsParsed = true;
  if (session.Tags.empty())
    return;

  cJSON* json = cJSON_Parse(session.Tags.c_str());
  if (!json)
    return;

  if (json->type == cJSON_Object)
  {
    cJSON* kernel = cJSON_GetObjectItem(json, "kernel");
    if (kernel && kernel->type == cJSON_String && kernel->valuestring && kernel->valuestring[0])
      session.Name = kernel->valuestring;

    cJSON* engines = cJSON_GetObjectItem(json, "engines");
    if (engines && engines->type == cJSON_Array)
    {
      session.Engines.clear();
      session.FileTypes.clear();
      for (cJSON* engine = engines->child; engine; engine = engine->next)
      {
        if (engine->type == cJSON_Object)
        {
          std::string curEngine;
          cJSON* einfo;
          einfo = cJSON_GetObjectItem(engine, "name");
          if (einfo && einfo->type == cJSON_String && einfo->valuestring && einfo->valuestring[0])
          {
            curEngine = einfo->valuestring;
            session.Engines.push_back(curEngine);
          }
          einfo = cJSON_GetObjectItem(engine, SessionRegistrar::fileTypesTag().c_str());
          if (einfo && einfo->type == cJSON_Array && einfo->child)
          {
            StringList fileTypes;
            smtk::io::LoadJSON::getStringArrayFromJSON(einfo, fileTypes);
            if (curEngine.empty())
              curEngine = "*";
            session.FileTypes[curEngine] = fileTypes;
          }
        }
      }
    }
  }

  cJSON_Delete(json);
}

/**\brief Register a session "type" for use by an application.
  *
  * The type (\a bname) must be unique.
  * To make session names unique the following naming scheme is used:
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
  * This class allows applications to query available sessions by
  * the types of files they can read, the name of the session
  * class, or by arbitrary tags associated with them identifying
  * configuration options.
  * Once a query identifies a suitable session, the application
  * can construct sessions of the the given type using the
  * constructor or by calling convenience routines such as
  * createSession and createSessionWithTags.
  *
  * Again, note that the session name \a bname must be unique;
  * only one constructor is stored per \a bname.
  * Remus and other remote sessions should include
  * disambiguating information in both \a bname and \a tags.
  * The tag is assumed to be a JSON string.
  * See the SMTK User's Guide for more information
  * on its structure.
  * An example tag might be<pre>
  * {"class":"remus",
  *  "server":"tcp://localhost",
  *  "site":"foo.kitware.com",
  *  "session":"cgm", "engine":"OCC",
  *  "filetypes":[".brep", ".occ", ".stl"],
  *  "filesys":"8dfcb75c-a3b5-4e10-a9dd-85623afe3372"}
  *  </pre>
  * for the \a bname
  * "smtk::model[cgm{OCC},foo.kitware.com]@tcp://localhost".
  *
  * The tag names in the example above should be present for
  * remus remote sessions, with "engine" being optional depending
  * on whether the session supports multiple modeling kernels.
  *
  * Because constructors are smtk::function instances,
  * setup can be included as part of the construction
  * process. For instance, the RemusRemoteSession class
  * calls registerSession multiple times with different
  * names and session types whenever a connection to a
  * new Remus server is opened: one registerSession call
  * for each remote session type. The constructor method
  * passed creates a RemusRemoteSession instance and
  * configures it to (1) use a given Remus server connection
  * and (2) prepare an actual session of the proper type
  * on the SMTK model worker at the far end of the server
  * connection.
  */
bool SessionRegistrar::registerSession(const std::string& bname, const std::string& btags,
  SessionStaticSetup bsetup, SessionConstructor bctor, OperatorConstructors* sopcons)
{
  if (!bname.empty() && bctor)
  {
    StaticSessionInfo entry(bname, btags, bsetup, bctor, sopcons);
    (*s_sessions())[bname] = entry;
    //std::cout << "Adding session " << bname << "\n";
    return true;
  }
  else if (!bname.empty() && s_sessions()->count(bname) > 0)
  { // unregister the session of the given name.
    s_sessions()->erase(bname);
    //std::cout << "Removing session " << bname << "\n";
    // FIXME: We should ensure that no registered sessions are of type bname.
    //        Presumably, by deleting all such sessions and removing their entities
    //        from storage.
  }
  return false;
}

/// Return a list of the names of each session subclass whose constructor has been registered with SMTK.
StringList SessionRegistrar::sessionTypeNames()
{
  StringList result;
  for (SessionConstructors::const_iterator it = s_sessions()->begin(); it != s_sessions()->end();
       ++it)
    result.push_back(it->first);
  return result;
}

/// Return the list of file types this session can read (currently: a list of file extensions).
StringData SessionRegistrar::sessionFileTypes(const std::string& bname, const std::string& bengine)
{
  SessionConstructors::iterator it = s_sessions()->find(bname);
  if (it != s_sessions()->end())
  {
    if (!it->second.TagsParsed)
      SessionRegistrar::parseTags(it->second);
    // when there is no engine passed in, we will return all
    // extensions from all engines
    PropertyNameWithStrings entry;
    StringData retFileTypes;
    if (bengine.empty() && !it->second.FileTypes.empty())
    {
      for (entry = it->second.FileTypes.begin(); entry != it->second.FileTypes.end(); ++entry)
      {
        retFileTypes[entry->first].insert(
          retFileTypes[entry->first].end(), entry->second.begin(), entry->second.end());
      }
    }
    else if (!bengine.empty())
    {
      if ((entry = it->second.FileTypes.find(bengine)) != it->second.FileTypes.end())
        retFileTypes[bengine].insert(
          retFileTypes[bengine].end(), entry->second.begin(), entry->second.end());
    }
    return retFileTypes;
  }
  StringData empty;
  return empty;
}

/**\brief Return the tag string that describes this session.
  *
  * The tag should be a JSON dictionary containing fields to present
  * to a user when selecting sessions.
  * The JSON may also be used to group sessions by capabilities.
  */
std::string SessionRegistrar::sessionTags(const std::string& bname)
{
  SessionConstructors::const_iterator it = s_sessions()->find(bname);
  if (it != s_sessions()->end())
    return it->second.Tags;
  std::string empty;
  return empty;
}

/**\brief Return the site that describes the filesystem for this session.
  *
  * The site should be a unique, user-presentable label identifying
  * the filesystem available to the model worker.
  * If empty, the site should be interpreted to be the root
  * filesystem of the local machine.
  */
std::string SessionRegistrar::sessionSite(const std::string& bname)
{
  SessionConstructors::iterator it = s_sessions()->find(bname);
  if (it != s_sessions()->end())
  {
    if (!it->second.TagsParsed)
      SessionRegistrar::parseTags(it->second);
    return it->second.Site;
  }
  std::string empty;
  return empty;
}

/**\brief Return the list of engines that the session provides.
  *
  * Some sessions, such as CGM, expose multiple solid modeling kernels;
  * in this case, we call the underlying kernels "engines" and
  * provide a way to obtain the list of engines for a given session name.
  */
StringList SessionRegistrar::sessionEngines(const std::string& bname)
{
  SessionConstructors::iterator it = s_sessions()->find(bname);
  if (it != s_sessions()->end())
  {
    if (!it->second.TagsParsed)
      SessionRegistrar::parseTags(it->second);
    return it->second.Engines;
  }
  StringList empty;
  return empty;
}

/**\brief Return a function to perform pre-construction Session setup for the given session type.
  *
  * A "null" method is returned if you pass an invalid session type name.
  *
  * \sa SessionStaticSetup
  */
SessionStaticSetup SessionRegistrar::sessionStaticSetup(const std::string& bname)
{
  SessionStaticSetup result = SMTK_FUNCTION_INIT;
  SessionConstructors::const_iterator it = s_sessions()->find(bname);
  if (it != s_sessions()->end())
    result = it->second.Setup;
  return result;
}

/**\brief Return a function to construct a Session instance given its class-specific name. Or NULL if you pass an invalid name.
  *
  * After calling the constructor, you most probably want to call
  * registerSession with the new session's UUID.
  */
SessionConstructor SessionRegistrar::sessionConstructor(const std::string& bname)
{
  SessionConstructor result = SMTK_FUNCTION_INIT;
  SessionConstructors::const_iterator it = s_sessions()->find(bname);
  if (it != s_sessions()->end())
    result = it->second.Constructor;
  return result;
}

OperatorConstructors* SessionRegistrar::sessionOperatorConstructors(const std::string& stype)
{
  OperatorConstructors* opcons = NULL;
  SessionConstructors::const_iterator it = s_sessions()->find(stype);
  if (it != s_sessions()->end())
  {
    opcons = it->second.OpConstructors;
  }
  return opcons;
}

/**\brief Return a set of operator names given a session typename.
  *
  */
std::set<std::string> SessionRegistrar::sessionOperatorNames(const std::string& stype)
{
  std::set<std::string> result;
  OperatorConstructors* opcons = SessionRegistrar::sessionOperatorConstructors(stype);
  if (opcons)
  {
    for (auto opit = opcons->begin(); opit != opcons->end(); ++opit)
    {
      result.insert(opit->first);
    }
  }
  return result;
}

/**\brief Return a new Session instance given its class-specific name. Or NULL if you pass an invalid name.
  *
  */
SessionPtr SessionRegistrar::createSession(const std::string& bname)
{
  SessionConstructor ctor = SMTK_FUNCTION_INIT;
  SessionConstructors::const_iterator it = s_sessions()->find(bname);
  if (it != s_sessions()->end())
    ctor = it->second.Constructor;
  if (ctor)
    return ctor();
  return SessionPtr();
}

} // model namespace
} // smtk namespace

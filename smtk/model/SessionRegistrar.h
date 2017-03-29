//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionRegistrar_h
#define __smtk_model_SessionRegistrar_h
#ifndef SHIBOKEN_SKIP

#include "smtk/Function.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/StringData.h"

#include <map>

namespace smtk {
  namespace model {

/**\brief A (generic) function-pointer to perform pre-construction session setup.
  *
  * When a session is registered (by calling SessionRegistrar::registerSession),
  * a function of this signature may be passed to it, indicating that this
  * function may be called before a session is constructed.
  *
  * The method may be called multiple times before a session instance is
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
  * The effect of setup option names is entirely dependent on the session
  * and thus methods to recover from an impossible configuration are left
  * to Session subclasses to implement.
  */
typedef smtk::function<
  int (const std::string&, const StringList& val)> SessionStaticSetup;

/// A (generic) function-pointer to construct a session instance.
typedef smtk::function<SessionPtr()> SessionConstructor;

/// A record associating session information with a constructor method.
struct StaticSessionInfo {
  std::string Name;
  SessionStaticSetup Setup;
  SessionConstructor Constructor;
  OperatorConstructors* OpConstructors;
  std::string Tags;
  bool TagsParsed;
  std::string Site;
  StringList Engines;
  StringData FileTypes;

  StaticSessionInfo() : TagsParsed(false) { }
  StaticSessionInfo(
    const std::string& bname,
    const std::string& btags,
    SessionStaticSetup bsetup,
    SessionConstructor bctor,
    OperatorConstructors* opctors)
    : Name(bname), Setup(bsetup), Constructor(bctor), OpConstructors(opctors), Tags(btags), TagsParsed(false)
    { }
};

/// A map of session names to constructors.
typedef std::map<std::string,StaticSessionInfo> SessionConstructors;

/**\brief A helper for sessions that do not perform static setup.
  *
  * If a session subclass does not require setup,
  * pass this to SessionRegistrar::registerSession().
  */
inline int SessionHasNoStaticSetup(
  const std::string&,
  const StringList&)
{
  return 1;
}

/**\brief A static class for holding information about sessions to modeling kernels.
  *
  * This class is not wrapped for use in Python because shiboken
  * cannot parse headers for boost::bind and boost::function.
  * Use the model manager to identify available sessions and create sessions
  * as Manager (a subclass of Manager) exposes methods that can be wrapped.
  */
class SMTKCORE_EXPORT SessionRegistrar
{
public:
  static bool registerSession(
    const std::string& bname,
    const std::string& tags,
    SessionStaticSetup bsetup,
    SessionConstructor bctor,
    OperatorConstructors* sopcons);
  static StringList sessionTypeNames();
  static std::string sessionTags(const std::string& bname);
  static std::string sessionSite(const std::string& bname);
  static StringList sessionEngines(const std::string& bname);
  static StringData sessionFileTypes(
    const std::string& bname, const std::string& engine = std::string());
  static SessionStaticSetup sessionStaticSetup(const std::string& bname);
  static SessionConstructor sessionConstructor(const std::string& bname);
  static OperatorConstructors* sessionOperatorConstructors(const std::string& stype);
  static std::set<std::string> sessionOperatorNames(const std::string& stype);
  static SessionPtr createSession(const std::string& bname);

  static std::string fileTypesTag() { return "filetypes"; }

protected:
  static void cleanupSessions();
  static void parseTags(StaticSessionInfo& session);

  static SessionConstructors* s_sessions(bool del = false);
};

  } // namespace model
} // namespace smtk

#endif // SHIBOKEN_SKIP
#endif // __smtk_model_SessionRegistrar_h

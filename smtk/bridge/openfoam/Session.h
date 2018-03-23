//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_openfoam_Session_h
#define __smtk_session_openfoam_Session_h

#include "smtk/bridge/openfoam/Exports.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Session.h"

namespace smtk
{
namespace model
{

class ArrangementHelper;
}
}

namespace smtk
{
namespace bridge
{
namespace openfoam
{

class SMTKOPENFOAMSESSION_EXPORT Session : public smtk::model::Session
{
public:
  smtkTypeMacro(smtk::bridge::openfoam::Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::bridge::openfoam::Session);
  smtkDeclareModelingKernel();

  virtual ~Session();

  const std::string& workingDirectory() const { return m_workingDirectory; }
  void setWorkingDirectory(const std::string wd) { m_workingDirectory = wd; }

  void createWorkingDirectory() const;
  void removeWorkingDirectory() const;
  bool workingDirectoryExists() const;

protected:
  friend class Operator;

  typedef smtk::model::SessionInfoBits SessionInfoBits;

  Session();

private:
  std::string m_workingDirectory;

  Session(const Session&);        // Not implemented.
  void operator=(const Session&); // Not implemented.
};

} // namespace openfoam
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_openfoam_Session_h

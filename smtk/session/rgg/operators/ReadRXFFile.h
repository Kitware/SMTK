//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_ReadRXFFile_h
#define __smtk_session_rgg_ReadRXFFile_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace rgg
{

/**\brief Load a rxl file into rgg session
  */
class SMTKRGGSESSION_EXPORT ReadRXFFile : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::ReadRXFFile);
  smtkCreateMacro(ReadRXFFile);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_ReadRXLFile_h

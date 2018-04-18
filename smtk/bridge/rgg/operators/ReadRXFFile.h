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

#include "smtk/bridge/rgg/Operator.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief Load a rxl file into rgg session
  */
class SMTKRGGSESSION_EXPORT ReadRXFFile : public Operator
{
public:
  smtkTypeMacro(ReadRXFFile);
  smtkCreateMacro(ReadRXFFile);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_ReadRXLFile_h

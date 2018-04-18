//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_RemoveMaterial_h
#define __smtk_session_rgg_RemoveMaterial_h

#include "smtk/bridge/rgg/Operator.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief Remove a custom material into rgg session
  */
class SMTKRGGSESSION_EXPORT RemoveMaterial : public Operator
{
public:
  smtkTypeMacro(RemoveMaterial);
  smtkCreateMacro(RemoveMaterial);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_RemoveMaterial_h

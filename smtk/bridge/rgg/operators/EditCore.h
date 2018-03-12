//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_EditCore_h
#define __smtk_session_rgg_EditCore_h

#include "smtk/bridge/rgg/Operator.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief Edit a rgg core. What happens beneath the cover is that smtk is editing
 * properties stored on the model for simplicity purpose since one rgg model
 * can only have one core.
  */
class SMTKRGGSESSION_EXPORT EditCore : public Operator
{
public:
  smtkTypeMacro(EditCore);
  smtkCreateMacro(EditCore);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_EditCore_h

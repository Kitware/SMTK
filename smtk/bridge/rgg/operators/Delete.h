// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_rgg_Delete_h
#define smtk_session_rgg_Delete_h

#include "smtk/bridge/rgg/Operator.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief Delete rgg entities(Ex pin, cell)
    */
class SMTKRGGSESSION_EXPORT Delete : public Operator
{
public:
  smtkTypeMacro(Delete);
  smtkCreateMacro(Delete);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // smtk_session_rgg_Delete_h

//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_ExportMaterialList_h
#define __smtk_session_rgg_ExportMaterialList_h

#include "smtk/bridge/rgg/Operator.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief export material list for RGG model to SON file
  */
class SMTKRGGSESSION_EXPORT ExportMaterialList : public Operator
{
public:
  smtkTypeMacro(ExportMaterialList);
  smtkCreateMacro(ExportMaterialList);
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

#endif // __smtk_session_rgg_ExportMaterialList_h

//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_EditAssembly_h
#define __smtk_session_rgg_EditAssembly_h

#include "smtk/bridge/rgg/operators/CreateAssembly.h"

namespace smtk
{
namespace model
{
class AuxiliaryGeometry;
class EntityRef;
}
}

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief edit a rgg assembly
  * The nuclear assembly is converted into an auxiliary geometry in smtk world. All
  * parameters are stored as properties.
  */
class SMTKRGGSESSION_EXPORT EditAssembly : public Operator
{
public:
  smtkTypeMacro(EditAssembly);
  smtkCreateMacro(EditAssembly);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_EditAssembly_h

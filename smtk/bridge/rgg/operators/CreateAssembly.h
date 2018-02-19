//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_CreateAssembly_h
#define __smtk_session_rgg_CreateAssembly_h

#include "smtk/bridge/rgg/Operator.h"

/**\brief a type to define a group used for rgg assembly
  */
#define SMTK_BRIDGE_RGG_ASSEMBLY "_rgg_assembly"

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

/**\brief create a rgg assembly
  * The nuclear assembly is converted into a group in smtk world. All
  * parameters are stored as properties.
  */
class SMTKRGGSESSION_EXPORT CreateAssembly : public Operator
{
public:
  smtkTypeMacro(CreateAssembly);
  smtkCreateMacro(CreateAssembly);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

  // If it's in create mode, it would populate the assembly with predefined properties
  static void populateAssembly(
    smtk::model::Operator* op, smtk::model::Group& assembly, bool createMode = false);

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_CreateAssembly_h

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

#include "smtk/session/rgg/Exports.h"

#include "smtk/operation/XMLOperation.h"

/**\brief a type to define a group used for rgg assembly
  */
#define SMTK_SESSION_RGG_ASSEMBLY "_rgg_assembly"

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
namespace session
{
namespace rgg
{

/**\brief create a rgg assembly
  * The nuclear assembly is converted into a group in smtk world. All
  * parameters are stored as properties.
  */
class SMTKRGGSESSION_EXPORT CreateAssembly : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::CreateAssembly);
  smtkCreateMacro(CreateAssembly);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  // If it's in create mode, it would populate the assembly with predefined properties
  static void populateAssembly(Operation*, smtk::model::Group&, bool createMode = false);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_CreateAssembly_h

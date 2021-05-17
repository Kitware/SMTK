//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Print_h
#define __smtk_session_mesh_Print_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief Print the underlying data of a mesh session entity.
  */
class SMTKMESHSESSION_EXPORT Print : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::mesh::Print);
  smtkCreateMacro(Print);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace mesh
} // namespace session
} // namespace smtk

#endif

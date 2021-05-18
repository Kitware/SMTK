//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_mesh_CreateUniformGrid_h
#define smtk_session_mesh_CreateUniformGrid_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief Construct a 2- or 3-dimensional uniform grid and its sides.
  */
class SMTKMESHSESSION_EXPORT CreateUniformGrid : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::mesh::CreateUniformGrid);
  smtkCreateMacro(CreateUniformGrid);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace mesh
} // namespace session
} // namespace smtk

#endif

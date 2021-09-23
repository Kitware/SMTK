//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_mesh_Write_h
#define smtk_session_mesh_Write_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief Write an smtk mesh model file.
  */
class SMTKMESHSESSION_EXPORT Write : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::mesh::Write);
  smtkCreateMacro(Write);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKMESHSESSION_EXPORT bool write(
  const smtk::resource::ResourcePtr&,
  const std::shared_ptr<smtk::common::Managers>&);
} // namespace mesh
} // namespace session
} // namespace smtk

#endif

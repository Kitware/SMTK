//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Import_h
#define __smtk_session_mesh_Import_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace mesh
{
class Read;

class SMTKMESHSESSION_EXPORT Import : public smtk::operation::XMLOperation
{
  friend class Read;

public:
  smtkTypeMacro(smtk::session::mesh::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
  const char* xmlDescription() const override;

  bool callFromRead = false;
};
} // namespace mesh
} // namespace session
} // namespace smtk

#endif

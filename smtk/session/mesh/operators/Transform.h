//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Transform_h
#define __smtk_session_mesh_Transform_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief Transform a model.
  */
class SMTKMESHSESSION_EXPORT Transform : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::mesh::Transform);
  smtkCreateMacro(Transform);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
}
}
}

#endif

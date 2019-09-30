//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Merge_h
#define __smtk_session_mesh_Merge_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief Merge entities from the same model and of like dimension into a single
   entity.
  */
class SMTKMESHSESSION_EXPORT Merge : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::mesh::Merge);
  smtkCreateMacro(Merge);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
}
}
}

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_mesh_Write_h
#define __smtk_session_mesh_Write_h

#include "smtk/bridge/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

/**\brief Write an smtk mesh model file.
  */
class SMTKMESHSESSION_EXPORT Write : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(Write);
  smtkCreateMacro(Write);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

SMTKMESHSESSION_EXPORT bool write(const smtk::resource::ResourcePtr&);
}
}
}

#endif

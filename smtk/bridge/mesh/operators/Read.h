//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_mesh_Read_h
#define __smtk_session_mesh_Read_h

#include "smtk/bridge/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

/**\brief Read an smtk mesh model file.
  */
class SMTKMESHSESSION_EXPORT Read : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

smtk::resource::ResourcePtr read(const std::string&);
}
}
}

#endif

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

#include "smtk/bridge/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

class SMTKMESHSESSION_EXPORT Import : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::bridge::mesh::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
  virtual const char* xmlDescription() const override;
};
}
}
}

#endif

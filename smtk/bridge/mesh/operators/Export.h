//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Export_h
#define __smtk_session_mesh_Export_h

#include "smtk/bridge/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

class SMTKMESHSESSION_EXPORT Export : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::bridge::mesh::Export);
  smtkCreateMacro(Export);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace mesh
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_mesh_Export_h

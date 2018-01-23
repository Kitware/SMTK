//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_ImportOperator_h
#define __smtk_session_mesh_ImportOperator_h

#include "smtk/bridge/mesh/Exports.h"

#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

class SMTKMESHSESSION_EXPORT ImportOperator : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(ImportOperator);
  smtkCreateMacro(ImportOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace mesh
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_mesh_ImportOperator_h

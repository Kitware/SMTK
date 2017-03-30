//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_ReadOperator_h
#define __smtk_session_mesh_ReadOperator_h

#include "smtk/bridge/mesh/Operator.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

class SMTKMESHSESSION_EXPORT ReadOperator : public Operator
{
public:
  smtkTypeMacro(ReadOperator);
  smtkCreateMacro(ReadOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace mesh
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_mesh_ReadOperator_h

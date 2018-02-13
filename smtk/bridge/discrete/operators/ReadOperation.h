//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_ReadOperation_h
#define __smtk_session_discrete_ReadOperation_h

#include "smtk/bridge/discrete/Operation.h"
#include "vtkCMBModelReadOperation.h"
#include "vtkNew.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

/**\brief Read a CMB discrete model file.
  *
  * This requires the file to be of type/extension "cmb" (which
  * is really just a VTK XML polydata file).
  */
class SMTKDISCRETESESSION_EXPORT ReadOperation : public Operation
{
public:
  smtkTypeMacro(ReadOperation);
  smtkCreateMacro(ReadOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  ReadOperation();
  Result operateInternal() override;
  const char* xmlDescription() const override;

  vtkNew<vtkCMBModelReadOperation> m_op;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_ReadOperation_h

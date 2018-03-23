//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_WriteOperation_h
#define __smtk_session_discrete_WriteOperation_h

#include "smtk/bridge/discrete/Operation.h"
#include "vtkCMBModelWriterBase.h"
#include "vtkNew.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

/**\brief Write a CMB discrete model file.
  *
  * This requires the file to be of type/extension "cmb".
  */
class SMTKDISCRETESESSION_EXPORT WriteOperation : public Operation
{
public:
  smtkTypeMacro(smtk::bridge::discrete::WriteOperation);
  smtkCreateMacro(WriteOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  WriteOperation();
  Result operateInternal() override;
  const char* xmlDescription() const override;

  vtkNew<vtkCMBModelWriterBase> m_op;
  int m_currentversion;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_WriteOperation_h

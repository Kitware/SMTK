//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_WriteOperator_h
#define __smtk_session_discrete_WriteOperator_h

#include "smtk/bridge/discrete/Operator.h"
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
class SMTKDISCRETESESSION_EXPORT WriteOperator : public Operator
{
public:
  smtkTypeMacro(WriteOperator);
  smtkCreateMacro(WriteOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(Operator);

  bool ableToOperate() override;

protected:
  WriteOperator();
  Result operateInternal() override;
  const char* xmlDescription() const override;

  vtkNew<vtkCMBModelWriterBase> m_op;
  int m_currentversion;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_WriteOperator_h

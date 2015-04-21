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

#include "smtk/bridge/discrete/Exports.h"
#include "smtk/model/Operator.h"
#include "vtkCMBModelWriterBase.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class Session;

/**\brief Write a CMB discrete model file.
  *
  * This requires the file to be of type/extension "cmb".
  */
class SMTKDISCRETESESSION_EXPORT WriteOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(WriteOperator);
  smtkCreateMacro(WriteOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  WriteOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Session* discreteSession() const;

  vtkNew<vtkCMBModelWriterBase> m_op;
  int m_currentversion;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_WriteOperator_h

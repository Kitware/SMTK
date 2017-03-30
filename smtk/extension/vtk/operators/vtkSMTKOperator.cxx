//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/operators/vtkSMTKOperator.h"
#include "smtk/model/Operator.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkSMTKOperator);

vtkSMTKOperator::vtkSMTKOperator()
{
}

vtkSMTKOperator::~vtkSMTKOperator()
{
}

void vtkSMTKOperator::SetSMTKOperator(smtk::model::OperatorPtr op)
{
  if (this->m_smtkOp.lock() != op)
  {
    this->m_smtkOp = op;
    this->Modified();
  }
}

smtk::model::OperatorPtr vtkSMTKOperator::GetSMTKOperator()
{
  return this->m_smtkOp.lock();
}

bool vtkSMTKOperator::AbleToOperate()
{
  return this->m_smtkOp.lock() ? this->m_smtkOp.lock()->ableToOperate() : false;
}

smtk::model::OperatorResult vtkSMTKOperator::Operate()
{
  return this->m_smtkOp.lock() ? this->m_smtkOp.lock()->operate() : smtk::model::OperatorResult();
}

void vtkSMTKOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "smtk op: " << (this->m_smtkOp.lock() ? this->m_smtkOp.lock()->name() : "(none)")
     << endl;

  this->Superclass::PrintSelf(os, indent);
}

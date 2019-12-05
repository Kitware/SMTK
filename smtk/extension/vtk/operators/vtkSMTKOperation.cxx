//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/operators/vtkSMTKOperation.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkSMTKOperation);

vtkSMTKOperation::vtkSMTKOperation() = default;

vtkSMTKOperation::~vtkSMTKOperation() = default;

void vtkSMTKOperation::SetSMTKOperation(smtk::operation::Operation::Ptr op)
{
  if (m_smtkOp.lock() != op)
  {
    m_smtkOp = op;
    this->Modified();
  }
}

smtk::operation::Operation::Ptr vtkSMTKOperation::GetSMTKOperation()
{
  return m_smtkOp.lock();
}

bool vtkSMTKOperation::AbleToOperate()
{
  return m_smtkOp.lock() ? m_smtkOp.lock()->ableToOperate() : false;
}

smtk::operation::Operation::Result vtkSMTKOperation::Operate()
{
  return m_smtkOp.lock() ? m_smtkOp.lock()->operate() : smtk::operation::Operation::Result();
}

void vtkSMTKOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "smtk op: " << (m_smtkOp.lock() ? m_smtkOp.lock()->typeName() : "(none)") << endl;

  this->Superclass::PrintSelf(os, indent);
}

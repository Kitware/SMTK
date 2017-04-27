//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/polygon/vtk/vtkSurfaceExtractorOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Operator.h"

#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkSurfaceExtractorOperator);

vtkSurfaceExtractorOperator::vtkSurfaceExtractorOperator()
{
}

vtkSurfaceExtractorOperator::~vtkSurfaceExtractorOperator()
{
}

bool vtkSurfaceExtractorOperator::AbleToOperate()
{
  bool able2Op = this->m_smtkOp.lock() &&
    this->m_smtkOp.lock()->name() == "extract surface contour" &&
    this->m_smtkOp.lock()->ensureSpecification();

  return able2Op;
}

smtk::model::OperatorResult vtkSurfaceExtractorOperator::Operate()
{
  // ONLY for create-edge-with-widget and edit-edge operations,
  if (!this->AbleToOperate())
  {
    return this->m_smtkOp.lock()->createResult(smtk::model::OPERATION_FAILED);
  }

  smtk::model::OperatorResult edgeResult;
  edgeResult = this->m_smtkOp.lock()->operate();

  return edgeResult;
}

void vtkSurfaceExtractorOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

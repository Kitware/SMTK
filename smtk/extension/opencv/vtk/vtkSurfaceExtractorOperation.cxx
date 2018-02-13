//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/opencv/vtk/vtkSurfaceExtractorOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Edge.h"

#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkSurfaceExtractorOperation);

vtkSurfaceExtractorOperation::vtkSurfaceExtractorOperation()
{
}

vtkSurfaceExtractorOperation::~vtkSurfaceExtractorOperation()
{
}

bool vtkSurfaceExtractorOperation::AbleToOperate()
{
  bool able2Op = this->m_smtkOp.lock() &&
    this->m_smtkOp.lock()->uniqueName() == "smtk::bridge::polygon::SurfaceExtractContours" &&
    this->m_smtkOp.lock()->ableToOperate();

  return able2Op;
}

smtk::operation::Operation::Result vtkSurfaceExtractorOperation::Operate()
{
  // ONLY for create-edge-with-widget and edit-edge operations,
  if (!this->AbleToOperate())
  {
    return this->m_smtkOp.lock()->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::operation::Operation::Result edgeResult;
  edgeResult = this->m_smtkOp.lock()->operate();

  return edgeResult;
}

void vtkSurfaceExtractorOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

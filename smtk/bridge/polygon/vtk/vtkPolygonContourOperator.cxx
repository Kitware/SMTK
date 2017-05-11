//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/polygon/vtk/vtkPolygonContourOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Operator.h"

#include "vtkBoundingBox.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include <vector>

vtkStandardNewMacro(vtkPolygonContourOperator);
vtkCxxSetObjectMacro(vtkPolygonContourOperator, ContourInput, vtkPolyData);
vtkPolygonContourOperator::vtkPolygonContourOperator()
{
  this->ContourInput = NULL;
  this->ImageBounds[0] = this->ImageBounds[2] = this->ImageBounds[4] = 0.0;
  this->ImageBounds[1] = this->ImageBounds[3] = this->ImageBounds[5] = -1.0;
}

vtkPolygonContourOperator::~vtkPolygonContourOperator()
{
  this->SetContourInput(NULL);
}

bool vtkPolygonContourOperator::AbleToOperate()
{
  bool able2Op = this->m_smtkOp.lock() && this->m_smtkOp.lock()->name() == "extract contours" &&
    this->m_smtkOp.lock()->ensureSpecification();
  if (!able2Op)
  {
    return able2Op;
  }

  //we need at least two point to create a valid edge
  able2Op = this->ContourInput != NULL && this->ContourInput->GetNumberOfLines() > 0 &&
    this->ContourInput->GetNumberOfPoints() >= 2;

  return able2Op;
}

smtk::model::OperatorResult vtkPolygonContourOperator::Operate()
{
  // ONLY for create-edge-with-widget and edit-edge operations,
  if (!this->AbleToOperate())
  {
    return this->m_smtkOp.lock()->createResult(smtk::model::OPERATION_FAILED);
  }

  smtk::model::OperatorResult edgeResult;
  smtk::attribute::AttributePtr spec = this->m_smtkOp.lock()->specification();
  smtk::attribute::IntItem::Ptr offsetsItem =
    spec->findAs<smtk::attribute::IntItem>("offsets", smtk::attribute::ALL_CHILDREN);
  smtk::attribute::DoubleItem::Ptr pointsItem =
    spec->findAs<smtk::attribute::DoubleItem>("points", smtk::attribute::ALL_CHILDREN);
  smtk::attribute::IntItem::Ptr numCoords =
    spec->findAs<smtk::attribute::IntItem>("coordinates", smtk::attribute::ALL_CHILDREN);
  numCoords->setValue(3); // number of elements in coordinates
  smtk::attribute::DoubleItem::Ptr boundsItem =
    spec->findAs<smtk::attribute::DoubleItem>("image bounds", smtk::attribute::ALL_CHILDREN);
  vtkBoundingBox imgbox(this->ImageBounds);
  if (imgbox.IsValid() && boundsItem)
  {
    std::vector<double> vec(this->ImageBounds, this->ImageBounds + 6);
    boundsItem->setValues(vec.begin(), vec.end());
  }

  double p[3];
  int numPoints = 0;
  vtkIdType *pts, npts;
  vtkCellArray* lines = this->ContourInput->GetLines();
  lines->InitTraversal();
  std::vector<int> offsets;
  while (lines->GetNextCell(npts, pts))
  {
    // for each line we are creating an edge, so set the "offsets" into the
    // points list
    offsets.push_back(numPoints);
    // add points for current line cell
    pointsItem->setNumberOfValues((numPoints + npts) * 3);
    for (vtkIdType j = 0; j < npts; ++j)
    {
      this->ContourInput->GetPoint(pts[j], p);
      int idx = 3 * (numPoints + j);
      for (int i = 0; i < 3; ++i)
      {
        pointsItem->setValue(idx + i, p[i]);
      }
    }
    numPoints += npts;
  }

  offsetsItem->setValues(offsets.begin(), offsets.end());
  edgeResult = this->m_smtkOp.lock()->operate();

  return edgeResult;
}

void vtkPolygonContourOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  if (this->ContourInput)
  {
    os << indent << "Contour Source::PrintSelf " << endl;
    this->ContourInput->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent << "Contour Source: (null)" << endl;
  }

  this->Superclass::PrintSelf(os, indent);
}

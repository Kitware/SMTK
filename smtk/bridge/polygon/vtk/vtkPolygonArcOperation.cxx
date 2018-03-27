//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/polygon/vtk/vtkPolygonArcOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Edge.h"

#include "vtkContourRepresentation.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkPolygonArcOperation);
vtkCxxSetObjectMacro(vtkPolygonArcOperation, ArcRepresentation, vtkContourRepresentation);

vtkPolygonArcOperation::vtkPolygonArcOperation()
{
  this->ArcRepresentation = NULL;
}

vtkPolygonArcOperation::~vtkPolygonArcOperation()
{
  this->SetArcRepresentation(NULL);
}

bool vtkPolygonArcOperation::AbleToOperate()
{
  bool able2Op = m_smtkOp.lock() &&
    (m_smtkOp.lock()->typeName() == "smtk::bridge::polygon::TweakEdge" ||
                   m_smtkOp.lock()->typeName() == "smtk::bridge::polygon::CreateEdge") &&
    m_smtkOp.lock()->ableToOperate();
  if (!able2Op)
  {
    return able2Op;
  }

  // for create-edge operation, we only handle "interactive widget" case
  if (m_smtkOp.lock()->typeName() == "smtk::bridge::polygon::CreateEdge")
  {
    smtk::attribute::IntItem::Ptr optypeItem =
      m_smtkOp.lock()->parameters()->findInt("construction method");
    able2Op = optypeItem && (optypeItem->discreteIndex(0) == 2);
  }
  if (!able2Op)
  {
    return able2Op;
  }

  // for create-edge-with-widget and edit-edge operation, we need arc source
  //we need at least two point to create a valid edge
  vtkPolyData* arcPoly =
    this->ArcRepresentation ? this->ArcRepresentation->GetContourRepresentationAsPolyData() : NULL;
  able2Op = arcPoly != NULL && arcPoly->GetNumberOfLines() > 0 && arcPoly->GetNumberOfPoints() >= 2;

  if (able2Op && m_smtkOp.lock()->typeName() == "smtk::bridge::polygon::TweakEdge")
  {
    smtk::model::Edge edge =
      m_smtkOp.lock()->parameters()->associations()->value().as<smtk::model::Edge>();
    able2Op = edge.isValid();
    ;
  }

  return able2Op;
}

smtk::operation::Operation::Result vtkPolygonArcOperation::Operate()
{
  // ONLY for create-edge-with-widget and edit-edge operations,
  if (!this->AbleToOperate())
  {
    return m_smtkOp.lock()->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::operation::Operation::Result edgeResult;
  smtk::attribute::AttributePtr spec = m_smtkOp.lock()->parameters();
  vtkPolyData* pd = this->ArcRepresentation->GetContourRepresentationAsPolyData();
  vtkCellArray* lines = pd->GetLines();

  bool isTweak = m_smtkOp.lock()->typeName() == "smtk::bridge::polygon::TweakEdge";
  spec->findAs<smtk::attribute::IntItem>("coordinates", smtk::attribute::ALL_CHILDREN)
    ->setValue(3); // number of coordinates per point
  smtk::attribute::DoubleItem::Ptr pointsItem =
    spec->findAs<smtk::attribute::DoubleItem>("points", smtk::attribute::ALL_CHILDREN);
  smtk::attribute::IntItem::Ptr offsetsItem = spec->findAs<smtk::attribute::IntItem>(
    isTweak ? "promote" : "offsets", smtk::attribute::ALL_CHILDREN);

  double p[3];
  int numPoints = 0;
  vtkIdType *pts, npts;
  lines->InitTraversal();
  while (lines->GetNextCell(npts, pts))
  {
    // add points for current line cell
    pointsItem->setNumberOfValues((numPoints + npts) * 3);
    for (vtkIdType j = 0; j < npts; ++j)
    {
      pd->GetPoint(pts[j], p);
      int idx = 3 * (numPoints + j);
      for (int i = 0; i < 3; ++i)
      {
        pointsItem->setValue(idx + i, p[i]);
      }
    }
    numPoints += npts;
  }

  std::vector<int> indices;
  if (!isTweak && !this->ArcRepresentation->GetNumberOfIntermediatePoints(0))
  { // We must indicate that the first edge starts at 0 even if it is not marked as a model edge
    indices.push_back(0);
  }
  int count = this->ArcRepresentation->GetNumberOfNodes();
  int offsets = 0;
  for (int i = 0; i < count; ++i, ++offsets) // ++offset for the node
  {
    offsets += this->ArcRepresentation->GetNumberOfIntermediatePoints(i);
    if (this->ArcRepresentation->GetNthNodeSelected(i))
    {
      indices.push_back(offsets);
    }
  }
  offsetsItem->setValues(indices.begin(), indices.end());

  edgeResult = m_smtkOp.lock()->operate();

  return edgeResult;
}

void vtkPolygonArcOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  if (this->ArcRepresentation)
  {
    os << indent << "Arc Source::PrintSelf " << endl;
    this->ArcRepresentation->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent << "Arc Source: (null)" << endl;
  }

  this->Superclass::PrintSelf(os, indent);
}

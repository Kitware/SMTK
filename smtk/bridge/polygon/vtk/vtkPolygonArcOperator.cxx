//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/polygon/vtk/vtkPolygonArcOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Operator.h"

#include "vtkContourRepresentation.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkPolygonArcOperator);

vtkPolygonArcOperator::vtkPolygonArcOperator()
{
  this->ArcRepresentation = NULL;
}

vtkPolygonArcOperator::~vtkPolygonArcOperator()
{
  this->SetArcRepresentation(NULL);
}

bool vtkPolygonArcOperator::AbleToOperate()
{
  bool able2Op = this->m_smtkOp.lock()
                 && (this->m_smtkOp.lock()->name() == "tweak edge"
                    || this->m_smtkOp.lock()->name() == "create edge")
                 && this->m_smtkOp.lock()->ensureSpecification()
                 ;
  if(!able2Op)
    {
    return able2Op;
    }

  // for create-edge operation, we only handle "interactive widget" case
  if(this->m_smtkOp.lock()->name() == "create edge")
    {
    smtk::attribute::IntItem::Ptr optypeItem =
      this->m_smtkOp.lock()->specification()->findInt("construction method");
    able2Op = optypeItem && (optypeItem->discreteIndex(0) == 2);
    }
  if(!able2Op)
    {
    return able2Op;
    }

  // for create-edge-with-widget and edit-edge operation, we need arc source
  //we need at least two point to create a valid edge
  vtkPolyData* arcPoly = this->ArcRepresentation ?
    this->ArcRepresentation->GetContourRepresentationAsPolyData() : NULL;
  able2Op = arcPoly != NULL
            && arcPoly->GetNumberOfLines() > 0
            && arcPoly->GetNumberOfPoints() >= 2
            ;

  if(able2Op && this->m_smtkOp.lock()->name() == "tweak edge")
    {
    smtk::model::Edge edge = this->m_smtkOp.lock()->specification()->
      associations()->value().as<smtk::model::Edge>();
    able2Op = edge.isValid();;
    }

  return able2Op;
}

smtk::model::OperatorResult vtkPolygonArcOperator::Operate()
{
  // ONLY for create-edge-with-widget and edit-edge operations,
  if(!this->AbleToOperate())
    {
    return this->m_smtkOp.lock()->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult edgeResult;
  smtk::attribute::AttributePtr spec = this->m_smtkOp.lock()->specification();
  vtkPolyData *pd = this->ArcRepresentation->GetContourRepresentationAsPolyData();
  vtkCellArray* lines = pd->GetLines();

  bool isTweak = this->m_smtkOp.lock()->name() == "tweak edge";
  spec->findAs<smtk::attribute::IntItem>("coordinates", smtk::attribute::ALL_CHILDREN)->setValue(3); // number of coordinates per point
  smtk::attribute::DoubleItem::Ptr pointsItem =
    spec->findAs<smtk::attribute::DoubleItem>("points", smtk::attribute::ALL_CHILDREN);
  smtk::attribute::IntItem::Ptr offsetsItem =
    spec->findAs<smtk::attribute::IntItem>(isTweak ? "promote" : "offsets", smtk::attribute::ALL_CHILDREN);

  double p[3];
  int numPoints = 0;
  vtkIdType *pts,npts;
  lines->InitTraversal();
  while(lines->GetNextCell(npts,pts))
    {
    // add points for current line cell
    pointsItem->setNumberOfValues((numPoints + npts) * 3);
    for (vtkIdType j=0; j < npts; ++j)
      {
      pd->GetPoint(pts[j],p);
      int idx = 3 * (numPoints+j);
      for (int i = 0; i < 3; ++i)
        {
        pointsItem->setValue( idx + i, p[i]);
        }
      }
    numPoints += npts;
    }

  std::vector<int> indices;
  if (!isTweak && !this->ArcRepresentation->GetNumberOfIntermediatePoints(0))
    { // We must indicate that the first edge starts at 0 even if it is not marked as a model edge
    indices.push_back(0);
    }
  int count = this->ArcRepresentation->GetNumberOfNodes() - 1;
  int offsets = 0;
  for ( int i = 0; i < count; ++i, ++offsets ) // ++offset for the node
    {
    offsets += this->ArcRepresentation->GetNumberOfIntermediatePoints(i);
    if(this->ArcRepresentation->GetNthNodeSelected(i))
      {
      indices.push_back(offsets);
      }
    }
  offsetsItem->setValues(indices.begin(), indices.end());

  edgeResult = this->m_smtkOp.lock()->operate();

  return edgeResult;
}

void vtkPolygonArcOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  if(this->ArcRepresentation)
    {
    os << indent << "Arc Source::PrintSelf " << endl;
    this->ArcRepresentation->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "Arc Source: (null)" << endl;
    }

  this->Superclass::PrintSelf(os,indent);
}

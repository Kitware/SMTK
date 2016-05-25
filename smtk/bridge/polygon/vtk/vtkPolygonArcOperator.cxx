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
#include "smtk/model/Operator.h"

#include "vtkContourRepresentation.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkPolygonArcOperator);

//----------------------------------------------------------------------------
vtkPolygonArcOperator::vtkPolygonArcOperator()
{
  this->ArcRepresentation = NULL;
}

//----------------------------------------------------------------------------
vtkPolygonArcOperator::~vtkPolygonArcOperator()
{
  this->SetArcRepresentation(NULL);
}

//----------------------------------------------------------------------------
bool vtkPolygonArcOperator::AbleToOperate()
{
  bool able2Op = this->m_smtkOp.lock()
                 && this->m_smtkOp.lock()->name() == "edit edge"
                 && this->m_smtkOp.lock()->ensureSpecification()
                 ;
  if(!able2Op)
    {
    return able2Op;
    }

  // for Create and Edit operation, we need arc source
  smtk::attribute::StringItem::Ptr optypeItem =
    this->m_smtkOp.lock()->specification()->findString("Operation");
  std::string optype = optypeItem->value();
  if(optype == "Create" || optype == "Edit")
    {
    //we need at least two point to create a valid edge
    vtkPolyData* arcPoly = this->ArcRepresentation ?
      this->ArcRepresentation->GetContourRepresentationAsPolyData() : NULL;
    able2Op = arcPoly != NULL
              && arcPoly->GetNumberOfLines() > 0
              && arcPoly->GetNumberOfPoints() >= 2
              ;
    }

  if(able2Op && optype == "Edit")
    {
    able2Op = this->m_smtkOp.lock()->specification()->findModelEntity("edge")
        ->value().isValid();;
    }

  return able2Op;
}

//----------------------------------------------------------------------------
smtk::model::OperatorResult vtkPolygonArcOperator::Operate()
{
  if(!this->AbleToOperate())
    {
    return this->m_smtkOp.lock()->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult edgeResult;
  smtk::attribute::AttributePtr spec = this->m_smtkOp.lock()->specification();
  vtkPolyData *pd = this->ArcRepresentation->GetContourRepresentationAsPolyData();
  vtkCellArray* lines = pd->GetLines();

  smtk::attribute::IntItem::Ptr offsetsItem = spec->findAs<smtk::attribute::IntItem>(
              "edge offsets", smtk::attribute::ALL_CHILDREN);
  smtk::attribute::DoubleItem::Ptr pointsItem = spec->findAs<smtk::attribute::DoubleItem>(
              "edge points", smtk::attribute::ALL_CHILDREN);
  smtk::attribute::StringItem::Ptr optypeItem =
    spec->findString("Operation");
  std::string optype = optypeItem->value();
  if(optype == "Create" || optype == "Edit")
    {
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

    // we skip the selected state of the first and last nodes in the contour,
    // because they should not be modified with "edit edge" operator. If users
    // do want to modify those vertices, they should do it with "merge edge" operator.
    int count = this->ArcRepresentation->GetNumberOfNodes() - 1;
    int offsets = 1;
    for ( int i = 1; i < count; ++i, ++offsets ) // ++offset for the node
      {
      offsets += this->ArcRepresentation->GetNumberOfIntermediatePoints(i);
      if(this->ArcRepresentation->GetNthNodeSelected(i))
        {
        offsetsItem->appendValue(offsets);
        }
      }

    edgeResult = this->m_smtkOp.lock()->operate();
    }
  else
    {
    edgeResult = this->m_smtkOp.lock()->createResult(smtk::model::OPERATION_FAILED);
    }

  return edgeResult;
}

//----------------------------------------------------------------------------
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

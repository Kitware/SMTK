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

#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkPolygonArcOperator);

//----------------------------------------------------------------------------
void polyLines2edgePoints(vtkPolyData *mesh,
                         smtk::attribute::DoubleItem::Ptr pointsItem,
                         vtkIdType *pts, vtkIdType npts)
{
  double p[3];
  // create edge for current line cell
  pointsItem->setNumberOfValues(npts * 3);
  for (vtkIdType j=0; j < npts; ++j)
    {
    mesh->GetPoint(pts[j],p);
    for (int i = 0; i < 3; ++i)
      {
      pointsItem->setValue(3 * j + i, p[i]);
      }
    }
}

//----------------------------------------------------------------------------
vtkPolygonArcOperator::vtkPolygonArcOperator()
{
  this->ArcSource = NULL;
}

//----------------------------------------------------------------------------
vtkPolygonArcOperator::~vtkPolygonArcOperator()
{
  this->SetArcSource(NULL);
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
    able2Op = this->ArcSource != NULL
              && this->ArcSource->GetNumberOfLines() > 0
              && this->ArcSource->GetNumberOfPoints() >= 2
              ;
    }

  return able2Op;
}

//----------------------------------------------------------------------------
smtk::model::OperatorResult vtkPolygonArcOperator::Operate()
{
  if(!this->AbleToOperate())
    {
    return smtk::model::OperatorResult();
    }

  smtk::model::OperatorResult edgeResult;
  smtk::attribute::AttributePtr spec = this->m_smtkOp.lock()->specification();
  smtk::attribute::StringItem::Ptr optypeItem =
    spec->findString("Operation");
  std::string optype = optypeItem->value();
  if(optype == "Create")
    {
    vtkPolyData *pd = this->ArcSource;
    vtkCellArray* lines = pd->GetLines();

    smtk::attribute::IntItem::Ptr offsetsItem = spec->findAs<smtk::attribute::IntItem>(
                "edge offsets", smtk::attribute::ALL_CHILDREN);
    smtk::attribute::DoubleItem::Ptr pointsItem = spec->findAs<smtk::attribute::DoubleItem>(
                "edge points", smtk::attribute::ALL_CHILDREN);
    int offsets = 0;
    vtkIdType *pts,npts;
    lines->InitTraversal();
    while(lines->GetNextCell(npts,pts))
      {
      // create edge for current line cell
      polyLines2edgePoints(pd, pointsItem, pts, npts);
      if(offsets > 0)
        offsetsItem->appendValue(offsets);
      offsets += npts;
      }

    edgeResult = this->m_smtkOp.lock()->operate();
    }

  return edgeResult;
}

//----------------------------------------------------------------------------
smtk::model::OperatorResult vtkPolygonArcOperator::Operate(vtkPolyData *source)
{
  this->SetArcSource(source);
  return this->Operate();
}

//----------------------------------------------------------------------------
void vtkPolygonArcOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  if(this->ArcSource)
    {
    os << indent << "Arc Source::PrintSelf " << endl;
    this->ArcSource->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "Arc Source: (null)" << endl;
    }

  this->Superclass::PrintSelf(os,indent);
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkADHExporterOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelGridRepresentation.h"
#include "vtkModelItemIterator.h"
#include <fstream>
#include <vtkCharArray.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>

namespace
{
// A mapping from VTK's tet face ordering to ADH's tet face
// ordering. Note that it also includes the c to fortran indexing.
int vtkToAdhFaceMapping[4] = { 3, 1, 2, 4 };
}

vtkStandardNewMacro(vtkADHExporterOperator);

vtkADHExporterOperator::vtkADHExporterOperator()
{
  this->OperateSucceeded = 0;
  this->ClientText = 0;
}

vtkADHExporterOperator::~vtkADHExporterOperator()
{
  this->SetClientText(0);
}

void vtkADHExporterOperator::Operate(vtkDiscreteModelWrapper* modelWrapper)
{
  if (!this->AbleToOperate(modelWrapper))
  {
    this->OperateSucceeded = 0;
    return;
  }
  vtkDiscreteModel* model = modelWrapper->GetModel();
  vtkModelGridRepresentation* analysisGridInfo = model->GetAnalysisGridInfo();
  if (analysisGridInfo == NULL)
  {
    vtkErrorMacro("Cannot create proper ADH input without analysis mesh.");
    this->OperateSucceeded = 0;
    return;
  }
  if (analysisGridInfo->IsModelConsistent(model) == false)
  {
    vtkErrorMacro("Analysis grid is not consistent with model.");
    this->OperateSucceeded = 0;
    return;
  }

  std::ofstream file(this->GetFileName());
  if (file.good() == 0)
  {
    file.close();
    vtkErrorMacro("Problem opening file.");
    this->OperateSucceeded = 0;
    return;
  }

  //the first thing we need to write is the client information
  file << this->ClientText;
  file.flush();

  vtkSmartPointer<vtkIdList> analysisGridIds = vtkSmartPointer<vtkIdList>::New();
  for (int i = 0; i < this->GetNumberOfAppliedNodalBCs(); i++)
  {
    int bcIndex, bcGroupType;
    vtkIdType bcsNodalGroupId;
    if (this->GetAppliedNodalBC(i, bcIndex, bcsNodalGroupId, bcGroupType))
    {
      if (analysisGridInfo->GetBCSNodalAnalysisGridPointIds(
            model, bcsNodalGroupId, bcGroupType, analysisGridIds))
      {
        for (vtkIdType j = 0; j < analysisGridIds->GetNumberOfIds(); j++)
        {
          file << "NDS " << analysisGridIds->GetId(j) + 1 << " " << bcIndex << endl;
        }
      }
      else
      {
        vtkErrorMacro("Problem outputting nodal group " << bcsNodalGroupId << " info.\n");
        this->OperateSucceeded = 0;
        return;
      }
    }
  }

  // I say face everywhere but really mean boundary (e.g. face in 3D, edge in 2D)
  // Note also that I add 1 to all of the indices since ADH wants a fortran
  // style indexing.
  vtkSmartPointer<vtkIdList> analysisCellSides = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdTypeArray> analysisCellEdges = vtkSmartPointer<vtkIdTypeArray>::New();
  for (int i = 0; i < this->GetNumberOfAppliedElementBCs(); i++)
  {
    int bcIndex;
    vtkIdType boundaryGroupId;
    this->GetAppliedElementBC(i, bcIndex, boundaryGroupId);

    if (model->GetModelDimension() == 3)
    {
      if (analysisGridInfo->GetBoundaryGroupAnalysisFacets(
            model, boundaryGroupId, analysisGridIds, analysisCellSides))
      {
        for (vtkIdType j = 0; j < analysisGridIds->GetNumberOfIds(); j++)
        {
          // side is in VTK face ordering with C indexing
          int side = static_cast<int>(analysisCellSides->GetId(j));
          // when outputting we need to change it to ADH ordering
          file << "FCS " << analysisGridIds->GetId(j) + 1 << " " << vtkToAdhFaceMapping[side] << " "
               << bcIndex << endl;
        }
      }
    }
    else if (model->GetModelDimension() == 2)
    {
      vtkDiscreteModelEntityGroup* boundaryGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
        model->GetModelEntity(vtkDiscreteModelEntityGroupType, boundaryGroupId));
      if (boundaryGroup == NULL)
      {
        vtkErrorMacro("Could not find boundary group.");
        this->OperateSucceeded = 0;
        return;
      }
      vtkSmartPointer<vtkModelItemIterator> edges;
      edges.TakeReference(boundaryGroup->NewModelEntityIterator());
      for (edges->Begin(); !edges->IsAtEnd(); edges->Next())
      {
        vtkModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
        if (analysisGridInfo->GetModelEdgeAnalysisPoints(
              model, edge->GetUniquePersistentId(), analysisCellEdges))
        {
          vtkIdType values[2];
          for (vtkIdType j = 0; j < analysisCellEdges->GetNumberOfTuples(); j++)
          {
            analysisCellEdges->GetTypedTuple(j, values);
            if (edge->GetNumberOfAdjacentModelFaces() >= 2)
            {
              file << "MDS " << values[0] + 1 << " " << values[1] + 1 << " " << bcIndex << endl;
            }
            else
            {
              file << "EGS " << values[0] + 1 << " " << values[1] + 1 << " " << bcIndex << endl;
            }
          }
        }
        else
        {
          vtkErrorMacro("Could not find model edge.");
          this->OperateSucceeded = 0;
          return;
        }
      }
    }
    else // analysisGridInfo->GetBoundaryGroupAnalysisFacets
    {
      vtkErrorMacro("Problem outputting boundary group " << boundaryGroupId << " info.\n");
      this->OperateSucceeded = 0;
    }
  }

  file << "END\n";
  file.close();
  this->OperateSucceeded = 1;
}

bool vtkADHExporterOperator::AbleToOperate(vtkDiscreteModelWrapper* modelWrapper)
{
  if (!modelWrapper)
  {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
  }
  return this->Superclass::AbleToOperate(modelWrapper->GetModel());
}

void vtkADHExporterOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}

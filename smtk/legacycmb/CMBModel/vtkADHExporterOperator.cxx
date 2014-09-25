/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkADHExporterOperator.h"

#include <fstream>
#include <vtkCharArray.h>
#include "vtkModelGridRepresentation.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItemIterator.h"
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>

namespace
{
  // A mapping from VTK's tet face ordering to ADH's tet face
  // ordering. Note that it also includes the c to fortran indexing.
  int vtkToAdhFaceMapping[4] = {3, 1, 2, 4};
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
  if(!this->AbleToOperate(modelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }
  vtkDiscreteModel* model = modelWrapper->GetModel();
  vtkModelGridRepresentation* analysisGridInfo = model->GetAnalysisGridInfo();
  if(analysisGridInfo == NULL)
    {
    vtkErrorMacro("Cannot create proper ADH input without analysis mesh.");
    this->OperateSucceeded = 0;
    return;
    }
  if(analysisGridInfo->IsModelConsistent(model) == false)
    {
    vtkErrorMacro("Analysis grid is not consistent with model.");
    this->OperateSucceeded = 0;
    return;
    }

  std::ofstream file(this->GetFileName());
  if(file.good() == 0)
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
  for(int i=0;i<this->GetNumberOfAppliedNodalBCs();i++)
    {
    int bcIndex, bcGroupType;
    vtkIdType bcsNodalGroupId;
    if(this->GetAppliedNodalBC(i, bcIndex, bcsNodalGroupId, bcGroupType))
      {
      if(analysisGridInfo->GetBCSNodalAnalysisGridPointIds(
        model, bcsNodalGroupId, bcGroupType,analysisGridIds))
        {
        for(vtkIdType j=0;j<analysisGridIds->GetNumberOfIds();j++)
          {
          file << "NDS " << analysisGridIds->GetId(j)+1 << " " << bcIndex << endl;
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
  for(int i=0;i<this->GetNumberOfAppliedElementBCs();i++)
    {
    int bcIndex;
    vtkIdType boundaryGroupId;
    this->GetAppliedElementBC(i, bcIndex, boundaryGroupId);

    if(model->GetModelDimension() == 3)
      {
      if(analysisGridInfo->GetBoundaryGroupAnalysisFacets(
           model, boundaryGroupId, analysisGridIds, analysisCellSides))
        {
        for(vtkIdType j=0;j<analysisGridIds->GetNumberOfIds();j++)
          {
          // side is in VTK face ordering with C indexing
          int side = static_cast<int>(analysisCellSides->GetId(j));
          // when outputting we need to change it to ADH ordering
          file << "FCS " << analysisGridIds->GetId(j)+1 <<  " "
               << vtkToAdhFaceMapping[side] << " " << bcIndex << endl;
          }
        }
      }
    else if(model->GetModelDimension() == 2)
      {
      vtkDiscreteModelEntityGroup* boundaryGroup =
        vtkDiscreteModelEntityGroup::SafeDownCast(
          model->GetModelEntity(vtkDiscreteModelEntityGroupType, boundaryGroupId));
      if(boundaryGroup == NULL)
        {
        vtkErrorMacro("Could not find boundary group.");
        this->OperateSucceeded = 0;
        return;
        }
      vtkSmartPointer<vtkModelItemIterator> edges;
      edges.TakeReference(boundaryGroup->NewModelEntityIterator());
      for(edges->Begin();!edges->IsAtEnd();edges->Next())
        {
        vtkModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
        if(analysisGridInfo->GetModelEdgeAnalysisPoints(model, edge->GetUniquePersistentId(), analysisCellEdges))
          {
          vtkIdType values[2];
          for(vtkIdType j=0;j<analysisCellEdges->GetNumberOfTuples();j++)
            {
            analysisCellEdges->GetTupleValue(j, values);
            if(edge->GetNumberOfAdjacentModelFaces() >= 2)
              {
              file << "MDS " << values[0]+1 << " " << values[1]+1 << " " << bcIndex << endl;
              }
            else
              {
              file << "EGS " << values[0]+1 << " " << values[1]+1 << " " << bcIndex << endl;
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
  if(!modelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(modelWrapper->GetModel());
}

void vtkADHExporterOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}

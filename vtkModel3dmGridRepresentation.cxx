/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed, or modified, in any form or by any means, without
permission in writing from Kitware Inc.

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
#include "vtkModel3dmGridRepresentation.h"

#include <vtkCharArray.h>
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include "vtkModelItemIterator.h"
#include "vtkNew.h"
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkModel3dmGridRepresentation);

//----------------------------------------------------------------------------
vtkModel3dmGridRepresentation::vtkModel3dmGridRepresentation()
{
  this->ModelPointToAnalysisPoint = vtkIdTypeArray::New();
  this->ModelCellToAnalysisCells = vtkIdTypeArray::New();
  this->ModelCellToAnalysisCellSides = vtkCharArray::New();
}

//----------------------------------------------------------------------------
vtkModel3dmGridRepresentation::~vtkModel3dmGridRepresentation()
{
  if(this->ModelPointToAnalysisPoint)
    {
    this->ModelPointToAnalysisPoint->Delete();
    this->ModelPointToAnalysisPoint = NULL;
    }
  if(this->ModelCellToAnalysisCells)
    {
    this->ModelCellToAnalysisCells->Delete();
    this->ModelCellToAnalysisCells = NULL;
    }
  if(this->ModelCellToAnalysisCellSides)
    {
    this->ModelCellToAnalysisCellSides->Delete();
    this->ModelCellToAnalysisCellSides = NULL;
    }
}

//----------------------------------------------------------------------------
bool vtkModel3dmGridRepresentation::GetBCSNodalAnalysisGridPointIds(
  vtkDiscreteModel* model, vtkIdType bcsGroupId,
  int bcGroupType, vtkIdList* pointIds)
{
  pointIds->Reset();
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }
  if(vtkPolyData::SafeDownCast(model->GetGeometry()) == NULL)
    {  // we're on the client and don't know this info
    return false;
    }

  //pointIds->SetNumberOfIds(
  //  this->ModelPointToAnalysisPoint->GetNumberOfTuples());

  if(vtkDiscreteModelEntityGroup* bcsNodalGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
    model->GetModelEntity(vtkDiscreteModelEntityGroupType, bcsGroupId)))
    {
    vtkModelItemIterator* iterFace=bcsNodalGroup->NewIterator(vtkModelFaceType);
    for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
      {
      vtkDiscreteModelFace* entity =
        vtkDiscreteModelFace::SafeDownCast(iterFace->GetCurrentItem());
      if(entity)
        {
        vtkNew<vtkIdList> newPointIds;
        if(bcGroupType == 1)// vtkSBBCInstance::enBCModelEntityAllNodesType)
          {
          entity->GetAllPointIds(newPointIds.GetPointer());
          }
        else if(bcGroupType == 2)//vtkSBBCInstance::enBCModelEntityBoundaryNodesType)
          {
          entity->GetBoundaryPointIds(newPointIds.GetPointer());
          }
        else if(bcGroupType == 3)//vtkSBBCInstance::enBCModelEntityInteriorNodesType)
          {
          entity->GetInteriorPointIds(newPointIds.GetPointer());
          }
        // Adding the new point ids
        for(vtkIdType i=0; i<newPointIds->GetNumberOfIds(); i++)
          {
          pointIds->InsertUniqueId(newPointIds->GetId(i));
          }
        }
      }
    iterFace->Delete();
   // pointIds->Squeeze();

    for(vtkIdType i=0;i<pointIds->GetNumberOfIds();i++)
      {
      vtkIdType analysisPointId =
        this->ModelPointToAnalysisPoint->GetValue(pointIds->GetId(i));
      pointIds->SetId(i, analysisPointId);
      }
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkModel3dmGridRepresentation::GetFloatingEdgeAnalysisGridPointIds(
  vtkDiscreteModel* model, vtkIdType nodalGroupId, vtkIdList* pointIds)
{
  vtkErrorMacro("3dm file does not support floating edges.");
  return false;
}

//----------------------------------------------------------------------------
bool vtkModel3dmGridRepresentation::GetModelEdgeAnalysisPoints(
  vtkDiscreteModel* model, vtkIdType boundaryGroupId, vtkIdTypeArray* edgePoints)
{
  vtkErrorMacro("Does not support 2D models.");
  return false;
}

//----------------------------------------------------------------------------
bool vtkModel3dmGridRepresentation::GetBoundaryGroupAnalysisFacets(
  vtkDiscreteModel* model, vtkIdType boundaryGroupId,
  vtkIdList* cellIds, vtkIdList* cellSides)
{
  cellIds->Reset();
  cellSides->Reset();
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }
  if(vtkPolyData::SafeDownCast(model->GetGeometry()) == NULL)
    {  // we're on the client and don't know this info
    return false;
    }
  if(vtkDiscreteModelEntityGroup* boundaryGroup =
     vtkDiscreteModelEntityGroup::SafeDownCast(
       model->GetModelEntity(vtkDiscreteModelEntityGroupType, boundaryGroupId)))
    {
    vtkModelItemIterator* entities = boundaryGroup->NewModelEntityIterator();
    for(entities->Begin();!entities->IsAtEnd();entities->Next())
      {
      if(vtkDiscreteModelGeometricEntity* entity =
         vtkDiscreteModelGeometricEntity::GetThisDiscreteModelGeometricEntity(
           vtkModelEntity::SafeDownCast(entities->GetCurrentItem())))
        {
        for(vtkIdType i=0;i<entity->GetNumberOfCells();i++)
          {
          // we only need (and return) one 3d cell and side that is adjacent
          // to the boundary facet
          vtkIdType masterCellId = entity->GetMasterCellId(i);
          vtkIdType id;
          this->ModelCellToAnalysisCells->GetTupleValue(masterCellId, &id);
          cellIds->InsertNextId(id);
          char side;
          this->ModelCellToAnalysisCellSides->GetTupleValue(masterCellId, &side);
          cellSides->InsertNextId(side);
          }
        }
      }
    entities->Delete();
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
bool vtkModel3dmGridRepresentation::IsModelConsistent(vtkDiscreteModel* model)
{
  if(vtkPolyData* poly = vtkPolyData::SafeDownCast(model->GetGeometry()))
    {  // we're on the server
    if(poly->GetNumberOfPoints() !=
       this->ModelPointToAnalysisPoint->GetNumberOfTuples())
      {
      vtkErrorMacro("Model does not match analysis grid.");
      this->Reset();
      return false;
      }
    if(poly->GetNumberOfCells() !=
       this->ModelCellToAnalysisCellSides->GetNumberOfTuples() ||
       poly->GetNumberOfCells() !=
       this->ModelCellToAnalysisCells->GetNumberOfTuples())
      {
      vtkErrorMacro("Model does not match analysis grid.");
      this->Reset();
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkModel3dmGridRepresentation::Initialize(
  const char* fileName, vtkDiscreteModel* model, vtkIdTypeArray* modelPointToAnalysisPoint,
    vtkIdTypeArray* modelCellToAnalysisCells, vtkCharArray* modelCellToAnalysisCellSides)
{
  this->SetGridFileName(fileName);
  this->ModelPointToAnalysisPoint->SetNumberOfTuples(
    modelPointToAnalysisPoint->GetNumberOfTuples());
  for(vtkIdType i=0;i<modelPointToAnalysisPoint->GetNumberOfTuples();i++)
    {
    vtkIdType value = modelPointToAnalysisPoint->GetValue(i);
    this->ModelPointToAnalysisPoint->SetValue(i, value);
    }
  vtkIdType numberOfCells = modelCellToAnalysisCells->GetNumberOfTuples();
  if(numberOfCells != modelCellToAnalysisCellSides->GetNumberOfTuples())
    {
    this->Reset();
    return false;
    }
  this->ModelCellToAnalysisCells->SetNumberOfTuples(numberOfCells);
  this->ModelCellToAnalysisCellSides->SetNumberOfTuples(numberOfCells);
  for(vtkIdType i=0;i<numberOfCells;i++)
    {
    vtkIdType value = modelCellToAnalysisCells->GetValue(i);
    this->ModelCellToAnalysisCells->SetValue(i, value);
    char cvalue = modelCellToAnalysisCellSides->GetValue(i);
    this->ModelCellToAnalysisCellSides->SetValue(i, cvalue);
    }

  return this->IsModelConsistent(model);
}

//----------------------------------------------------------------------------
void vtkModel3dmGridRepresentation::Reset()
{
  this->Superclass::Reset();
  this->ModelPointToAnalysisPoint->SetNumberOfTuples(0);
  this->ModelCellToAnalysisCells->SetNumberOfTuples(0);
  this->ModelCellToAnalysisCellSides->SetNumberOfTuples(0);
}

//----------------------------------------------------------------------------
void vtkModel3dmGridRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ModelPointToAnalysisPoint: "
     << this->ModelPointToAnalysisPoint << "\n";
  os << indent << "ModelCellToAnalysisCells: "
     << this->ModelCellToAnalysisCells << "\n";
  os << indent << "ModelCellToAnalysisCellSides: "
     << this->ModelCellToAnalysisCellSides << "\n";
}


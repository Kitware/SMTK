//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBParserBase.h"

#include "vtkDiscreteModel.h"
#include "vtkModelEntity.h"
#include "vtkModel3dmGridRepresentation.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelEdge.h"

#include <vtkCharArray.h>
#include <vtkFieldData.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>

vtkCMBParserBase::vtkCMBParserBase()
{
}

vtkCMBParserBase:: ~vtkCMBParserBase()
{
}

//-----------------------------------------------------------------------------
void vtkCMBParserBase::SetGeometry(vtkDiscreteModel* Model, vtkObject* Geometry)
{
  //we need an efficient way to DeepCopy just the structure of the geometry.
  //CopyStructure does a shallow copy, and DeepCopy will copy field data.
  //So what we can do is do a CopyStructure first, and DeepCopy that
  //to only deep copy the structure

  vtkNew<vtkPolyData> copiedStructure;
  copiedStructure->CopyStructure(vtkDataSet::SafeDownCast(Geometry));

  vtkNew<vtkPolyData> deepCopy;
  deepCopy->DeepCopy(copiedStructure.GetPointer());

  DiscreteMesh mesh(deepCopy.GetPointer());
  Model->SetMesh(mesh);
}

//-----------------------------------------------------------------------------
bool vtkCMBParserBase::AddCellsToGeometry(vtkDiscreteModelGeometricEntity* Entity,
                                         vtkIdList* MasterCellIds)
{
  return Entity->AddCellsToGeometry(MasterCellIds);
}

//-----------------------------------------------------------------------------
void vtkCMBParserBase::SetUniquePersistentId(vtkModelEntity* Entity,
                                             vtkIdType Id)
{
  Entity->SetUniquePersistentId(Id);
}

//-----------------------------------------------------------------------------
void vtkCMBParserBase::SetLargestUsedUniqueId(vtkModel* Model, vtkIdType MaxId)
{
  if(Model->GetLargestUsedUniqueId() < MaxId)
    {
    Model->SetLargestUsedUniqueId(MaxId);
    }
}

//----------------------------------------------------------------------------
// copied from vtkXMLUnstructuredDataReader.cxx
template <class TIn, class TOut>
void vtkCMBParserBaseCopyArray(TIn* in, TOut* out,
                               vtkIdType length)
{
  for(vtkIdType i = 0; i < length; ++i)
    {
    out[i] = static_cast<TOut>(in[i]);
    }
}

//-----------------------------------------------------------------------------
void vtkCMBParserBase::SetAnalysisGridInfo(
  vtkDiscreteModel* model, vtkDataArray* pointMapArray, vtkDataArray* cellMapArray,
  vtkCharArray* canonicalSideArray)
{
  vtkSmartPointer<vtkIdTypeArray> pMapArray = vtkIdTypeArray::SafeDownCast(pointMapArray);
  if(pMapArray == NULL)
    {
    pMapArray = vtkSmartPointer<vtkIdTypeArray>::New();
    pMapArray->SetName(pointMapArray->GetName());
    pMapArray->SetNumberOfComponents(pointMapArray->GetNumberOfComponents());
    vtkIdType numberOfTuples = pointMapArray->GetNumberOfTuples();
    pMapArray->SetNumberOfTuples(numberOfTuples);
    switch (pointMapArray->GetDataType())
      {
      vtkTemplateMacro(
        vtkCMBParserBaseCopyArray(
          static_cast<VTK_TT*>(pointMapArray->GetVoidPointer(0)),
          pMapArray->GetPointer(0), numberOfTuples));
      default:
        vtkErrorMacro("Cannot convert vtkDataArray of type "
                      << pointMapArray->GetDataType()
                      << " to vtkIdTypeArray.");
      }
    }

  vtkSmartPointer<vtkIdTypeArray> cMapArray = vtkIdTypeArray::SafeDownCast(cellMapArray);
  if(cMapArray == NULL)
    {
    cMapArray = vtkSmartPointer<vtkIdTypeArray>::New();
    cMapArray->SetName(cellMapArray->GetName());
    cMapArray->SetNumberOfComponents(cellMapArray->GetNumberOfComponents());
    vtkIdType numberOfTuples = cellMapArray->GetNumberOfTuples();
    cMapArray->SetNumberOfTuples(numberOfTuples);
    switch (cellMapArray->GetDataType())
      {
      vtkTemplateMacro(
        vtkCMBParserBaseCopyArray(
          static_cast<VTK_TT*>(cellMapArray->GetVoidPointer(0)),
          cMapArray->GetPointer(0), numberOfTuples));
      default:
        vtkErrorMacro("Cannot convert vtkDataArray of type "
                      << cellMapArray->GetDataType()
                      << " to vtkIdTypeArray.");
      }
    }

  vtkModel3dmGridRepresentation* analysisGridInfo =
    vtkModel3dmGridRepresentation::New();
  if(analysisGridInfo->Initialize(NULL, model, pMapArray, cMapArray,
                                  canonicalSideArray) == true)
    {
    model->SetAnalysisGridInfo(analysisGridInfo);
    }
  else
    {
    vtkErrorMacro("Problem reading information about analysis grid.");
    }
  analysisGridInfo->Delete();
}

//----------------------------------------------------------------------------
vtkIdTypeArray* vtkCMBParserBase::NewIdTypeArray(vtkDataArray* a)
{
  if(!a)
    {
    return 0;
    }
  // If it is already a vtkIdTypeArray, just return it after increasing
  // the reference count.
  vtkIdTypeArray* ida = vtkIdTypeArray::SafeDownCast(a);
  if(ida)
    {
    ida->Register(this);
    return ida;
    }

  // Need to convert the data.
  ida = vtkIdTypeArray::New();
  ida->SetNumberOfComponents(a->GetNumberOfComponents());
  ida->SetNumberOfTuples(a->GetNumberOfTuples());
  vtkIdType length = a->GetNumberOfComponents() * a->GetNumberOfTuples();
  vtkIdType* idBuffer = ida->GetPointer(0);
  switch (a->GetDataType())
    {
    vtkTemplateMacro(
      vtkCMBParserBaseCopyArray(
                      static_cast<VTK_TT*>(a->GetVoidPointer(0)),
                      idBuffer, length));
    default:
      vtkErrorMacro("Cannot convert vtkDataArray of type " << a->GetDataType()
                    << " to vtkIdTypeArray.");
      ida->Delete();
      ida = 0;
    }
  return ida;
}

//-----------------------------------------------------------------------------
void vtkCMBParserBase::SeparateCellClassification(vtkDiscreteModel* model,
                                vtkIdTypeArray* cellClassification,
                                vtkCMBParserBase::CellToModelType& cellToModelMap) const

{
  //we need to count the number of edges in our data set.
  //than convert each edge to be a negative value, and go from 0 to num edges
  //convert each face to go from 0 to num faces,
  vtkIdType counts[2] = {0,0}; //0 is non edges, 1 is edges
  for(vtkIdType i=0;i<cellClassification->GetNumberOfTuples();i++)
    {
    const vtkIdType value = cellClassification->GetValue(i);
    const bool isEdge =     (NULL != vtkDiscreteModelEdge::SafeDownCast(
                             model->GetModelEntity(vtkModelEdgeType, value)) );

    const vtkIdType meshId = counts[isEdge];
    ++counts[isEdge]; //increment counts for edges or faces

    CellToModelIterator it = cellToModelMap.find(value);
    if(it!=cellToModelMap.end())
      {
      it->second->InsertNextId(meshId);
      }
    else
      {
      vtkSmartPointer<vtkIdList> meshIdList = vtkSmartPointer<vtkIdList>::New();
      meshIdList->InsertNextId(meshId);
      cellToModelMap.insert(
          std::pair<vtkIdType, vtkSmartPointer<vtkIdList> >(value,meshIdList));
      }
    }
}

//-----------------------------------------------------------------------------
void vtkCMBParserBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

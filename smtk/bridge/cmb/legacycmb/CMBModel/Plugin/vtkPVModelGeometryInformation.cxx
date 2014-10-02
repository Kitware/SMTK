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
#include "vtkPVModelGeometryInformation.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkCellData.h"
#include "vtkClientServerStream.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkInformation.h"

#include "vtkIdTypeArray.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelGeometricEntity.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkBoundingBox.h"

vtkStandardNewMacro(vtkPVModelGeometryInformation);

//----------------------------------------------------------------------------
vtkPVModelGeometryInformation::vtkPVModelGeometryInformation()
{
  this->NumberOfPoints = 0;
  this->NumberOfCells = 0;
}

//----------------------------------------------------------------------------
vtkPVModelGeometryInformation::~vtkPVModelGeometryInformation()
{
  this->CellIdsMap.clear();
  this->EnityIdsMap.clear();
}

//----------------------------------------------------------------------------
void vtkPVModelGeometryInformation::CopyFromObject(vtkObject* obj)
{
  this->CellIdsMap.clear();
  this->EnityIdsMap.clear();
  vtkMath::UninitializeBounds(this->Bounds);

  vtkDataObject *dataObject = vtkDataObject::SafeDownCast( obj );

  // Handle the case where the a vtkAlgorithmOutput is passed instead of
  // the data object. vtkSMPart uses vtkAlgorithmOutput.
  if (!dataObject)
    {
    vtkAlgorithmOutput* algOutput = vtkAlgorithmOutput::SafeDownCast( obj );
    if (algOutput && algOutput->GetProducer())
      {
      dataObject = algOutput->GetProducer()->GetOutputDataObject(
        algOutput->GetIndex() );
      }
    vtkAlgorithm* alg = vtkAlgorithm::SafeDownCast( obj );
    if (alg)
      {
      dataObject = alg->GetOutputDataObject( 0 );
      }
    if (!dataObject)
      {
      vtkErrorMacro("Unable to get data object from object!");
      return;
      }
    }

  vtkBoundingBox bbox;
  vtkDiscreteModelWrapper* modelData = vtkDiscreteModelWrapper::SafeDownCast(dataObject);
  vtkDataSet* modelEntPoly=NULL;
  if (modelData)
    {
    vtkCompositeDataIterator* iter = modelData->NewIterator();
    iter->InitTraversal();
    vtkIdType numCells=0;
    while (!iter->IsDoneWithTraversal())
      {
      modelEntPoly = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (modelEntPoly)
        {
        numCells += modelEntPoly->GetNumberOfCells();
        unsigned int curr_idx = iter->GetCurrentFlatIndex();
        this->CellIdsMap[curr_idx] = vtkIdTypeArray::SafeDownCast(
          modelEntPoly->GetCellData()->GetArray(
          vtkDiscreteModelGeometricEntity::GetReverseClassificationArrayName()));
        vtkIdType entId;
        if (modelData->GetEntityIdByChildIndex(curr_idx-1, entId))
          {
          this->EnityIdsMap[curr_idx] = entId;
          }
        double bounds[6];
        modelEntPoly->GetBounds(bounds);
        bbox.AddBounds(bounds);
        }
      iter->GoToNextItem();
      }
    iter->Delete();
    this->NumberOfPoints = modelData->GetNumberOfPoints();
    this->NumberOfCells = numCells;
    bbox.GetBounds(this->Bounds);
    }
  else if (vtkDataSet::SafeDownCast(dataObject))
    {
    modelEntPoly = vtkDataSet::SafeDownCast(dataObject);
    this->NumberOfPoints = modelEntPoly->GetNumberOfPoints();
    this->NumberOfCells = modelEntPoly->GetNumberOfCells();
    this->CellIdsMap[0] = vtkIdTypeArray::SafeDownCast(
      modelEntPoly->GetCellData()->GetArray(
      vtkDiscreteModelGeometricEntity::GetReverseClassificationArrayName()));
    modelEntPoly->GetBounds(this->Bounds);
    }
  else
    {
    this->NumberOfPoints = 0;
    this->NumberOfCells = 0;
    vtkErrorMacro("Unrecognized data object for getting info!");
    }
}

//----------------------------------------------------------------------------
// Return a pointer to the geometry bounding box in the form
// (xmin,xmax, ymin,ymax, zmin,zmax).
double *vtkPVModelGeometryInformation::GetBounds()
{
  return this->Bounds;
}

//----------------------------------------------------------------------------
void vtkPVModelGeometryInformation::GetBounds(double bounds[6])
{
  for (int i=0; i<6; i++)
    {
    bounds[i] = this->Bounds[i];
    }
}
//----------------------------------------------------------------------------
int vtkPVModelGeometryInformation::GetMasterCellId(
  unsigned int flatidx,int idx)
{
  if(this->CellIdsMap.find(flatidx) != this->CellIdsMap.end())
    {
    vtkIdTypeArray* cellIdMapArray = this->CellIdsMap[flatidx];
    if(cellIdMapArray && cellIdMapArray->GetNumberOfTuples()>idx)
      {
      return cellIdMapArray->GetValue(idx);
      }
    }
  return -1;
}
//----------------------------------------------------------------------------
vtkIdType vtkPVModelGeometryInformation::GetModelEntityId(
  unsigned int flatidx)
{
  if(this->EnityIdsMap.find(flatidx) != this->EnityIdsMap.end())
    {
    return this->EnityIdsMap[flatidx];
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkPVModelGeometryInformation::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfPoints: " << this->NumberOfPoints << endl;
  os << indent << "NumberOfCells: " << this->NumberOfCells << endl;
}
//----------------------------------------------------------------------------
void
vtkPVModelGeometryInformation::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << this->NumberOfPoints << this->NumberOfCells <<
    vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void
vtkPVModelGeometryInformation::CopyFromStream(const vtkClientServerStream* css)
{
  css->GetArgument(0, 0, &this->NumberOfPoints);
  css->GetArgument(0, 1, &this->NumberOfCells);
}

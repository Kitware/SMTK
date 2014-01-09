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
#include "vtkCmbMeshToModelReader.h"

#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"

#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLDataElement.h"
#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkModelBCGridRepresentation.h"
#include "vtkCmbMeshGridRepresentationServer.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkCMBParserBase.h"
#include "vtkIdList.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "CmbFaceMeshHelper.h"

#include <iomanip>
#include <sstream>
#include <vtksys/SystemTools.hxx>


using namespace CmbFaceMesherClasses;

vtkStandardNewMacro(vtkCmbMeshToModelReader);
vtkCxxSetObjectMacro(vtkCmbMeshToModelReader, ModelWrapper, vtkDiscreteModelWrapper);

//----------------------------------------------------------------------------
// copied from vtkXMLUnstructuredDataReader.cxx
template <class TIn, class TOut>
void MeshToModelCopyArray(TIn* in, TOut* out,
                               vtkIdType length)
{
  for(vtkIdType i = 0; i < length; ++i)
    {
    out[i] = static_cast<TOut>(in[i]);
    }
}

//----------------------------------------------------------------------------
vtkCmbMeshToModelReader::vtkCmbMeshToModelReader()
{
  this->SetNumberOfInputPorts(0);
  this->ModelWrapper = 0;
  this->ModelDimension = 0;
  this->AnalysisGridFileName = NULL;
}

//----------------------------------------------------------------------------
vtkCmbMeshToModelReader::~vtkCmbMeshToModelReader()
{
  this->SetModelWrapper(0);
  this->SetAnalysisGridFileName(NULL);
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::GetDataSetMajorVersion()
{
  return 1;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::GetDataSetMinorVersion()
{
  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkCmbMeshToModelReader::GetDataSetName()
{
  return "PolyData";
}
//----------------------------------------------------------------------------
void vtkCmbMeshToModelReader::SetupEmptyOutput()
{
  this->GetCurrentOutput()->Initialize();
}
//----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::CanReadFileVersion(int major, int minor)
{
  if (major == 1 && minor==0)
    {
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::ReadPrimaryElement(vtkXMLDataElement* ePrimary)
{
  if(!ePrimary || !this->vtkXMLReader::ReadPrimaryElement(ePrimary))
    {
    return 0;
    }
  ePrimary->GetScalarAttribute("ModelDimension",
    this->ModelDimension);
  this->SetAnalysisGridFileName(ePrimary->GetAttribute(
    "AnalysisMeshFileName"));
  return 1;
}
//----------------------------------------------------------------------------
void vtkCmbMeshToModelReader::ReadXMLData()
{
  if(!this->ModelWrapper || !this->ModelWrapper->GetModel())
    {
    vtkWarningMacro("There is no model set for the reader.");
    this->DataError = 1;
    return;
    }

  // See if there is a FieldData element. There should be one, and only one for now
  if(!this->FieldDataElement)
    {
    vtkWarningMacro("There is no FieldData in the file.");
    this->DataError = 1;
    return;
    }

  // read the field data information
  int i, numTuples;
  vtkFieldData *fieldData = this->GetCurrentOutput()->GetFieldData();
  for(i=0; i < this->FieldDataElement->GetNumberOfNestedElements() &&
    !this->AbortExecute; i++)
    {
    vtkXMLDataElement* eNested = this->FieldDataElement->GetNestedElement(i);
    vtkAbstractArray* array = this->CreateArray(eNested);
    if (array)
      {
      if(eNested->GetScalarAttribute("NumberOfTuples", numTuples))
        {
        array->SetNumberOfTuples(numTuples);
        }
      else
        {
        numTuples = 0;
        }
      fieldData->AddArray(array);
      array->Delete();
      if (!this->ReadArrayValues(eNested, 0, array, 0,
       numTuples*array->GetNumberOfComponents()))
        {
        this->DataError = 1;
        return;
        }
      }
    }
  if(!this->LoadAnalysisGridInfo(fieldData))
    {
    this->DataError = 1;
    }
}
//-----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::LoadAnalysisGridInfo(vtkFieldData* fieldData)
{
  if(this->ModelDimension == 2)
    {
    return this->Load2DAnalysisGridInfo(fieldData);
    }
  else if(this->ModelDimension == 3)
    {
    return this->Load3DAnalysisGridInfo(fieldData);
    }
  return 0;
}
//----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::FillOutputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

//-----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::Load2DAnalysisGridInfo(vtkFieldData* fieldData)
{
  vtkDiscreteModel* model = this->ModelWrapper->GetModel();

  vtkDataArray* cellids =
    fieldData->GetArray(ModelFaceRep::Get2DAnalysisCellModelIdsString());
  vtkDataArray* ptsids =
    fieldData->GetArray(ModelFaceRep::Get2DAnalysisPointModelIdsString());
  vtkDataArray *cellPointIds =
    fieldData->GetArray(ModelFaceRep::Get2DAnalysisCellPointIdsString());
  if (ptsids && cellids && cellPointIds)
    {
    vtkCmbMeshGridRepresentationServer* gridRepresentationInfo =
      vtkCmbMeshGridRepresentationServer::New();
    vtkPolyData* gridRepPoly = vtkPolyData::New();
    gridRepPoly->Initialize();
    vtkIdTypeArray* newptsids = this->NewIdTypeArray(ptsids);
    vtkIdTypeArray* newcellids = this->NewIdTypeArray(cellids);
    vtkIdTypeArray* newcellptsids = this->NewIdTypeArray(cellPointIds);

    gridRepPoly->GetFieldData()->AddArray(newptsids);
    gridRepPoly->GetFieldData()->AddArray(newcellids);
    gridRepPoly->GetFieldData()->AddArray(newcellptsids);
    gridRepresentationInfo->Initialize(gridRepPoly, model);
    model->SetAnalysisGridInfo(gridRepresentationInfo);
    newptsids->Delete();
    newcellids->Delete();
    newcellptsids->Delete();
    gridRepPoly->Delete();
    gridRepresentationInfo->Delete();
    return 1;
    }
  else
    {
    vtkWarningMacro("There seems to be some information missing for"
      << " mapping back to the analysis mesh.");
    }
  return 0;
}
//-----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::Load3DAnalysisGridInfo(vtkFieldData* fieldData)
{
  vtkDiscreteModel* model = this->ModelWrapper->GetModel();
  vtkDataArray* bcFloatingEdgeData =
  fieldData->GetArray(vtkCMBParserBase::GetBCFloatingEdgeDataString());
  vtkDataArray* bcModelFaceData =
  fieldData->GetArray(vtkCMBParserBase::GetBCModelFaceDataString());
  // The model face mapping to analysis mesh must be present
  if(bcModelFaceData)
    {
    vtkModelBCGridRepresentation* gridRepresentation =
      vtkModelBCGridRepresentation::New();
    vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
    vtkIdType counter = 0;
    if(bcFloatingEdgeData)
      {
      // parse the floating edge data
      vtkIdTypeArray* idTypeArray = this->NewIdTypeArray(bcFloatingEdgeData);
      while(counter < idTypeArray->GetNumberOfTuples()*idTypeArray->GetNumberOfComponents())
        {
        vtkIdType floatingEdgeId = idTypeArray->GetValue(counter++);
        vtkIdType numberOfIds = idTypeArray->GetValue(counter++);
        ids->SetNumberOfIds(numberOfIds);
        for(vtkIdType id=0;id<numberOfIds;id++)
          {
          ids->InsertId(id, idTypeArray->GetValue(counter++));
          }
        if(gridRepresentation->AddFloatingEdge(floatingEdgeId, ids, model) == false)
          {
          idTypeArray->Delete();
          gridRepresentation->Delete();
          return 0;
          }
        }
      idTypeArray->Delete();
      }

    // parse the model face data
    counter = 0;
    vtkIdTypeArray* mfidTypeArray = this->NewIdTypeArray(bcModelFaceData);
    vtkSmartPointer<vtkIdList> cellSides = vtkSmartPointer<vtkIdList>::New();
    while(counter < mfidTypeArray->GetNumberOfTuples()*mfidTypeArray->GetNumberOfComponents())
      {
      vtkIdType modelFaceId = mfidTypeArray->GetValue(counter++);
      vtkIdType numberOfIds = mfidTypeArray->GetValue(counter++);
      ids->SetNumberOfIds(numberOfIds);
      cellSides->SetNumberOfIds(numberOfIds);
      for(vtkIdType id=0;id<numberOfIds;id++)
        {
        ids->InsertId(id, mfidTypeArray->GetValue(counter++));
        cellSides->InsertId(id, mfidTypeArray->GetValue(counter++));
        }
      if(gridRepresentation->AddModelFace(modelFaceId, ids, cellSides, model) == false)
        {
        mfidTypeArray->Delete();
        gridRepresentation->Delete();
        return 0;
        }
      }
    gridRepresentation->SetGridFileName(this->AnalysisGridFileName);
    model->SetAnalysisGridInfo(gridRepresentation);
    mfidTypeArray->Delete();
    gridRepresentation->Delete();
    return 1;
    }
  else
    {
    vtkWarningMacro("There seems to be some information missing for"
      << " mapping back to the analysis mesh.");
    }

  return 0;
}
// copied from vtkCMBParserBase.cxx
//----------------------------------------------------------------------------
vtkIdTypeArray* vtkCmbMeshToModelReader::NewIdTypeArray(vtkDataArray* a)
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
  ida->SetName(a->GetName());
  vtkIdType length = a->GetNumberOfComponents() * a->GetNumberOfTuples();
  vtkIdType* idBuffer = ida->GetPointer(0);
  void* inIdBuffer = a->GetVoidPointer(0);
  switch (a->GetDataType())
    {
    vtkTemplateMacro(
      MeshToModelCopyArray(
      static_cast<VTK_TT*>(inIdBuffer),
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
void vtkCmbMeshToModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

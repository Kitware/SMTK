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
#include "vtkCmbBCGridRepresentation.h"
#include "vtkCmbMeshGridRepresentationServer.h"
#include "vtkCMBModel.h"
#include "vtkCMBModelWrapper.h"
#include "vtkCMBParserBase.h"
#include "vtkIdList.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "CmbFaceMeshHelper.h"

#include <iomanip>
#include <sstream>
#include <vtksys/SystemTools.hxx>

using namespace vtkstd;
using namespace CmbFaceMesherClasses;

vtkStandardNewMacro(vtkCmbMeshToModelReader);
vtkCxxRevisionMacro(vtkCmbMeshToModelReader, "");
vtkCxxSetObjectMacro(vtkCmbMeshToModelReader, ModelWrapper, vtkCMBModelWrapper);

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
}

//----------------------------------------------------------------------------
vtkCmbMeshToModelReader::~vtkCmbMeshToModelReader()
{
  this->SetModelWrapper(0);
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
  return "MeshToModelMapFile";
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
  vtkstd::string strVersion = ePrimary->GetAttribute("Version");
  int modelDimension=0;
  ePrimary->GetScalarAttribute("ModelDimension", modelDimension);
  vtkstd::string strAnalysisMeshFileName = ePrimary->GetAttribute(
    "AnalysisMeshFileName");

  // See if there is a FieldData element. There should be one, and only one for now
  int numNested = ePrimary->GetNumberOfNestedElements();
  for(int i=0; i < numNested; ++i)
    {
    vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
    if(strcmp(eNested->GetName(), "FieldData") == 0)
      {
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
          if (!this->ReadArrayValues(eNested, 0, array, 0, numTuples*array->GetNumberOfComponents()))
            {
            this->DataError = 1;
            return 0;
            }
          }
        }
      return this->LoadAnalysisGridInfo(fieldData, modelDimension,
        strAnalysisMeshFileName.c_str());
      }
    }

  return 0;
}
//-----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::LoadAnalysisGridInfo(
  vtkFieldData* fieldData, int modelDim, const char* meshFileName)
{
  if(modelDim == 2)
    {
    return this->Load2DAnalysisGridInfo(fieldData, meshFileName);
    }
  else if(modelDim == 3)
    {
    return this->Load2DAnalysisGridInfo(fieldData, meshFileName);
    }
  return 0;
}
//-----------------------------------------------------------------------------
int vtkCmbMeshToModelReader::Load2DAnalysisGridInfo(
  vtkFieldData* fieldData, const char* meshFileName)
{
  if(!this->ModelWrapper || !this->ModelWrapper->GetModel())
    {
    vtkWarningMacro("There is no model set for the reader.");
    return 0;
    }
  vtkCMBModel* model = this->ModelWrapper->GetModel();

  vtkDataArray* cellids =
    fieldData->GetArray(ModelFaceRep::Get2DAnalysisCellModelIdsString());
  vtkDataArray* celltypes =
    fieldData->GetArray(ModelFaceRep::Get2DAnalysisCellModelTypesString());
  vtkDataArray* ptsids =
  fieldData->GetArray(ModelFaceRep::Get2DAnalysisPointModelIdsString());
  vtkDataArray* ptstypes =
  fieldData->GetArray(ModelFaceRep::Get2DAnalysisPointModelTypesString());
  if(cellids && celltypes && ptsids && ptstypes)
    {
    vtkNew<vtkCmbMeshGridRepresentationServer> gridRepresentationInfo;
    vtkNew<vtkPolyData> gridRepPoly;
    gridRepPoly->Initialize();
    gridRepPoly->GetFieldData()->ShallowCopy(fieldData);
    gridRepresentationInfo->SetRepresentation(gridRepPoly.GetPointer());
    model->SetAnalysisGridInfo(gridRepresentationInfo.GetPointer());
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
int vtkCmbMeshToModelReader::Load3DAnalysisGridInfo(
  vtkFieldData* fieldData, const char* meshFileName)
{
  if(!this->ModelWrapper || !this->ModelWrapper->GetModel())
    {
    vtkWarningMacro("There is no model set for the reader.");
    return 0;
    }
  vtkCMBModel* model = this->ModelWrapper->GetModel();
  vtkDataArray* bcFloatingEdgeData =
  fieldData->GetArray(vtkCMBParserBase::GetBCFloatingEdgeDataString());
  vtkDataArray* bcModelFaceData =
  fieldData->GetArray(vtkCMBParserBase::GetBCModelFaceDataString());
  if(bcFloatingEdgeData && bcModelFaceData)
    {
    vtkSmartPointer<vtkCmbBCGridRepresentation> gridRepresentation =
      vtkSmartPointer<vtkCmbBCGridRepresentation>::New();

    // parse the floating edge data
    vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
    vtkIdType counter = 0;
    vtkSmartPointer<vtkIdTypeArray> idTypeArray;
    idTypeArray.TakeReference(this->NewIdTypeArray(bcFloatingEdgeData));
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
        return 0;
        }
      }

    // parse the model face data
    counter = 0;
    idTypeArray.TakeReference(this->NewIdTypeArray(bcModelFaceData));
    vtkSmartPointer<vtkIdList> cellSides = vtkSmartPointer<vtkIdList>::New();
    while(counter < idTypeArray->GetNumberOfTuples()*idTypeArray->GetNumberOfComponents())
      {
      vtkIdType modelFaceId = idTypeArray->GetValue(counter++);
      vtkIdType numberOfIds = idTypeArray->GetValue(counter++);
      ids->SetNumberOfIds(numberOfIds);
      cellSides->SetNumberOfIds(numberOfIds);
      for(vtkIdType id=0;id<numberOfIds;id++)
        {
        ids->InsertId(id, idTypeArray->GetValue(counter++));
        cellSides->InsertId(id, idTypeArray->GetValue(counter++));
        }
      if(gridRepresentation->AddModelFace(modelFaceId, ids, cellSides, model) == false)
        {
        return 0;
        }
      }
    if(meshFileName && *meshFileName)
      {
      gridRepresentation->SetGridFileName(meshFileName);
      }
    model->SetAnalysisGridInfo(gridRepresentation);
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
  vtkIdType length = a->GetNumberOfComponents() * a->GetNumberOfTuples();
  vtkIdType* idBuffer = ida->GetPointer(0);
  void* inIdBuffer = a->GetVoidPointer(0);
  switch (a->GetDataType())
    {
    vtkTemplateMacro(
      MeshToModelCopyArray(
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
void vtkCmbMeshToModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

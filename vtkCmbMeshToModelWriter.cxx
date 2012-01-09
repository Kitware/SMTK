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
#include "vtkCmbMeshToModelWriter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkCMBMaterial.h"
#include "vtkCMBModel.h"
#include "vtkCMBModelEdge.h"
#include "vtkCMBModelEntityGroup.h"
#include "vtkCMBModelFace.h"
#include "vtkCMBModelRegion.h"
#include "vtkCMBModelWrapper.h"
#include "vtkCMBNodalGroup.h"
#include "vtkCMBUserName.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#
#include "vtkStringArray.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"
#include "vtkCmbBCGridRepresentation.h"
#include "vtkNew.h"
#include "vtkFieldData.h"
#include "vtkCmbMeshGridRepresentationServer.h"
#include "CmbFaceMeshHelper.h"
#include "vtkCMBParserBase.h"

#include <iomanip>
#include <sstream>
#include <vtksys/SystemTools.hxx>

using namespace std;
using namespace CmbFaceMesherClasses;

namespace {
  void CreateIdMap(vtkIdList* ids, std::map<int, int>& createdMap)
    {
    createdMap.clear();
    for(int i=0;i<ids->GetNumberOfIds();i++)
      {
      createdMap[ids->GetId(i)] = i;
      }
    }
  } // end local namespace

vtkStandardNewMacro(vtkCmbMeshToModelWriter);
vtkCxxRevisionMacro(vtkCmbMeshToModelWriter, "");
vtkCxxSetObjectMacro(vtkCmbMeshToModelWriter, ModelWrapper, vtkCMBModelWrapper);

//----------------------------------------------------------------------------
vtkCmbMeshToModelWriter::vtkCmbMeshToModelWriter()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(0);
  this->DataMode = vtkXMLWriter::Binary;
  this->ModelWrapper = 0;
}

//----------------------------------------------------------------------------
vtkCmbMeshToModelWriter::~vtkCmbMeshToModelWriter()
{
  this->SetModelWrapper(0);
}

//----------------------------------------------------------------------------
const char* vtkCmbMeshToModelWriter::GetDefaultFileExtension()
{
  return "m2m";
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelWriter::GetDataSetMajorVersion()
{
  return 1;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelWriter::GetDataSetMinorVersion()
{
  return 0;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelWriter::WriteData()
{
  int result = 0;
  // Write the file.
  if (!this->StartFile())
    {
    return result;
    }

  vtkIndent indent;
  result = this->WriteHeader(&indent);
  if(!result)
    {
    return result;
    }
  vtkIndent indent2 = indent.GetNextIndent();
  vtkCMBModel* Model = this->ModelWrapper->GetModel();
  int modelDim = Model->GetModelDimension();
  if(modelDim == 3)
    {
    result = this->Write3DModelMeshInfo(&indent2);
    }
  else if(modelDim == 2)
    {
    result = this->Write2DModelMeshInfo(&indent2);
    }
  if(result)
    {
    result = this->WriteFooter(&indent);
    }
  if (result)
    {
    result = this->EndFile();
    }

  return result;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelWriter::WriteHeader(vtkIndent* parentIndent)
{
  vtkCMBModel* Model = this->ModelWrapper->GetModel();
  vtkCmbGridRepresentation* gridRepresentation =
  Model->GetAnalysisGridInfo();
  if(gridRepresentation == NULL)
    {
    return 0;
    }

  const char*  analysisGridName = gridRepresentation->GetGridFileName();
  if(!analysisGridName || !(*analysisGridName))
    {
    vtkWarningMacro("This vtkCmbGridRepresentation does not have a GridFileName.");
    return 0;
    }

  ostream& os = *(this->Stream);
  vtkIndent indent = parentIndent->GetNextIndent();

  // Open the primary element.
  os << indent << "<" << this->GetDataSetName();
  // version
  std::stringstream version;
  version << this->GetDataSetMajorVersion() << "."
          << this->GetDataSetMinorVersion();
  os << " version=\"" << version.str().c_str() << "\"";
  // analysis grid name
  os << " AnalysisMeshFileName=\""
     <<vtksys::SystemTools::GetFilenameName(
     std::string(analysisGridName)).c_str() << "\"";
  // model dimension
  std::stringstream modelDim;
  modelDim << Model->GetModelDimension();
  os << " ModelDimension=\"" << modelDim.str().c_str() << "\">\n";

  // Close primary element:
  os.flush();
  if (os.fail())
    {
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelWriter::WriteFooter(vtkIndent* parentIndent)
{
  vtkIndent indent = parentIndent->GetNextIndent();
  ostream& os = *(this->Stream);

  // Close the primary element.
  os << indent << "</" << this->GetDataSetName() << ">\n";
  os.flush();
  if (os.fail())
    {
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelWriter::Write3DModelMeshInfo(vtkIndent* parentindent)
{
  vtkCMBModel* Model = this->ModelWrapper->GetModel();
  vtkCmbGridRepresentation* gridRepresentation =
    Model->GetAnalysisGridInfo();
  if(gridRepresentation == NULL)
    {
    return 0;
    }
  vtkCmbBCGridRepresentation* analysisGridInfo =
    vtkCmbBCGridRepresentation::SafeDownCast(gridRepresentation);
  if(!analysisGridInfo ||
    analysisGridInfo->IsModelConsistent(Model) == false)
    {
    vtkWarningMacro("Only the vtkCmbBCGridRepresentation, which is created from an "
      << "Omicron bc file, is supported currently for 3D model.");
    return 0;
    }

  vtkNew<vtkPolyData> tmpPoly;
  tmpPoly->Initialize();
  // the analysis grid info came from a bc file
  vtkIdList* idList = vtkIdList::New();
  // format is floating edge id, number of point ids, point ids,...
  if(Model->GetNumberOfAssociations(vtkModelEdgeType))
    {
    vtkIdTypeArray* floatingEdgeData = vtkIdTypeArray::New();
    vtkModelItemIterator* edges = Model->NewIterator(vtkModelEdgeType);
    vtkIdType floatingEdgeCounter = 0;
    for(edges->Begin();!edges->IsAtEnd();edges->Next())
      {
      vtkCMBModelEdge* edge =
        vtkCMBModelEdge::SafeDownCast(edges->GetCurrentItem());
      if(edge->GetModelRegion())
        {
        floatingEdgeCounter++;
        floatingEdgeData->InsertNextValue(edge->GetUniquePersistentId());
        analysisGridInfo->GetFloatingEdgeAnalysisGridPointIds(
          Model, edge->GetUniquePersistentId(), idList);
        floatingEdgeData->InsertNextValue(idList->GetNumberOfIds());
        for(vtkIdType i=0;i<idList->GetNumberOfIds();i++)
          {
          floatingEdgeData->InsertNextValue(idList->GetId(i));
          }
        }
      }
    floatingEdgeData->SetName(vtkCMBParserBase::GetBCFloatingEdgeDataString());
    tmpPoly->GetFieldData()->AddArray(floatingEdgeData);
    floatingEdgeData->Delete();
    edges->Delete();
    }

  vtkIdTypeArray* modelFaceData = vtkIdTypeArray::New();
  // format is model face id, number of facets, cell 0, side 0, cell 1, side 1,...
  vtkModelItemIterator* faces = Model->NewIterator(vtkModelFaceType);
  vtkIdList* cellSides = vtkIdList::New();
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkCMBModelFace* face =
      vtkCMBModelFace::SafeDownCast(faces->GetCurrentItem());
    modelFaceData->InsertNextValue(face->GetUniquePersistentId());
    analysisGridInfo->GetModelFaceAnalysisFacets(
      Model, face->GetUniquePersistentId(), idList, cellSides);
    modelFaceData->InsertNextValue(idList->GetNumberOfIds());
    for(vtkIdType i=0;i<idList->GetNumberOfIds();i++)
      {
      modelFaceData->InsertNextValue(idList->GetId(i));
      modelFaceData->InsertNextValue(cellSides->GetId(i));
      }
    }
  faces->Delete();
  modelFaceData->SetName(vtkCMBParserBase::GetBCModelFaceDataString());
  tmpPoly->GetFieldData()->AddArray(modelFaceData);
  modelFaceData->Delete();
  cellSides->Delete();
  idList->Delete();
  vtkFieldData* fieldData = tmpPoly->GetFieldData();
  vtkIndent indent2 = parentindent->GetNextIndent();
  if(this->DataMode == vtkXMLWriter::Appended)
    {
    this->WriteFieldDataAppended(fieldData, indent2, this->FieldDataOM);
    }
  else
    {
    // Write the point data arrays.
    this->WriteFieldDataInline(fieldData, indent2);
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkCmbMeshToModelWriter::Write2DModelMeshInfo(vtkIndent* parentindent)
{
  vtkCMBModel* Model = this->ModelWrapper->GetModel();
  vtkCmbGridRepresentation* gridRepresentation =
  Model->GetAnalysisGridInfo();
  if(gridRepresentation == NULL)
    {
    return 0;
    }
  vtkCmbMeshGridRepresentationServer* analysisGridInfo =
    vtkCmbMeshGridRepresentationServer::SafeDownCast(gridRepresentation);
  if(!analysisGridInfo || !analysisGridInfo->GetRepresentation() ||
    analysisGridInfo->IsModelConsistent(Model) == false)
    {
    vtkWarningMacro("Only the writing of vtkCmbMeshGridRepresentationServer "
      << "is supported currently for 2D model.");
    return 0;
    }
  vtkPolyData* gridPoly = analysisGridInfo->GetRepresentation();
  vtkIdTypeArray *ptsids = analysisGridInfo->GetPointIdMapArray();
  vtkIdTypeArray *cellids = analysisGridInfo->GetCellIdMapArray();
  if (!ptsids || !cellids)
    {
    vtkWarningMacro("One of the points or cell mapping arrays is missing.");
    return 0;
    }

  vtkIdType npts,*pts,i=0;
  vtkIdTypeArray *cellPointIds=NULL;
  vtkIdType numCells = gridPoly->GetNumberOfCells();
  if(numCells>0)
    {
    cellPointIds = vtkIdTypeArray::New();
    cellPointIds->SetNumberOfComponents(3);
    cellPointIds->SetNumberOfTuples(numCells);
    cellPointIds->SetName(
      ModelFaceRep::Get2DAnalysisCellPointIdsString());
    vtkIdType *cpd = reinterpret_cast<vtkIdType *>(
      cellPointIds->GetVoidPointer(0));
    vtkCellArray *polys = gridPoly->GetPolys();
    polys->InitTraversal();
    while(polys->GetNextCell(npts,pts) != NULL)
      {
      for(int j=0;j<3;j++)
        {
        *cpd = pts[j];
        cpd++;
        }
      }
    }
  else if(analysisGridInfo->GetCellPointIdsArray())
    {
    cellPointIds = analysisGridInfo->GetCellPointIdsArray();
    }
  else
    {
    vtkWarningMacro("The 2DAnalysisCellPointIds array is missing.");
    return 0;
    }

  vtkNew<vtkPolyData> tmpPoly;
  tmpPoly->Initialize();
  tmpPoly->GetFieldData()->AddArray(ptsids);
  tmpPoly->GetFieldData()->AddArray(cellids);
  tmpPoly->GetFieldData()->AddArray(cellPointIds);
  if(numCells>0 && cellPointIds)
    {
    cellPointIds->Delete();
    }
  vtkFieldData* fieldData = tmpPoly->GetFieldData();
  vtkIndent indent2 = parentindent->GetNextIndent();
  if(this->DataMode == vtkXMLWriter::Appended)
    {
    this->WriteFieldDataAppended(fieldData, indent2, this->FieldDataOM);
    }
  else
    {
    // Write the point data arrays.
    this->WriteFieldDataInline(fieldData, indent2);
    }

  return 1;
}

//-----------------------------------------------------------------------------
const char* vtkCmbMeshToModelWriter::GetDataSetName()
{
  return "PolyData";
}

//-----------------------------------------------------------------------------
void vtkCmbMeshToModelWriter::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ModelWrapper: " << this->ModelWrapper << "\n";
  }

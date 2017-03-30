//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshToModelWriter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkModelItemIterator.h"
#include "vtkModelMaterial.h"
#include "vtkModelUserName.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#
#include "cmbFaceMeshHelper.h"
#include "vtkCMBMeshGridRepresentationServer.h"
#include "vtkCMBParserBase.h"
#include "vtkFieldData.h"
#include "vtkModelBCGridRepresentation.h"
#include "vtkNew.h"
#include "vtkStringArray.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"

#include <iomanip>
#include <sstream>
#include <vtksys/SystemTools.hxx>

using namespace discreteFaceMesherClasses;

namespace
{
void CreateIdMap(vtkIdList* ids, std::map<int, int>& createdMap)
{
  createdMap.clear();
  for (int i = 0; i < ids->GetNumberOfIds(); i++)
  {
    createdMap[ids->GetId(i)] = i;
  }
}
} // end local namespace

vtkStandardNewMacro(vtkCMBMeshToModelWriter);
vtkCxxSetObjectMacro(vtkCMBMeshToModelWriter, ModelWrapper, vtkDiscreteModelWrapper);

vtkCMBMeshToModelWriter::vtkCMBMeshToModelWriter()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(0);
  this->DataMode = vtkXMLWriter::Binary;
  this->ModelWrapper = 0;
}

vtkCMBMeshToModelWriter::~vtkCMBMeshToModelWriter()
{
  this->SetModelWrapper(0);
}

const char* vtkCMBMeshToModelWriter::GetDefaultFileExtension()
{
  return "m2m";
}

int vtkCMBMeshToModelWriter::GetDataSetMajorVersion()
{
  return 1;
}

int vtkCMBMeshToModelWriter::GetDataSetMinorVersion()
{
  return 0;
}

int vtkCMBMeshToModelWriter::WriteData()
{
  int result = 0;
  // Write the file.
  if (!this->StartFile())
  {
    return result;
  }

  vtkIndent indent;
  result = this->WriteHeader(&indent);
  if (!result)
  {
    return result;
  }
  vtkIndent indent2 = indent.GetNextIndent();
  vtkDiscreteModel* Model = this->ModelWrapper->GetModel();
  int modelDim = Model->GetModelDimension();
  if (modelDim == 3)
  {
    result = this->Write3DModelMeshInfo(&indent2);
  }
  else if (modelDim == 2)
  {
    result = this->Write2DModelMeshInfo(&indent2);
  }
  if (result)
  {
    result = this->WriteFooter(&indent);
  }
  if (result)
  {
    result = this->EndFile();
  }

  return result;
}

int vtkCMBMeshToModelWriter::WriteHeader(vtkIndent* parentIndent)
{
  vtkDiscreteModel* Model = this->ModelWrapper->GetModel();
  vtkModelGridRepresentation* gridRepresentation = Model->GetAnalysisGridInfo();
  if (gridRepresentation == NULL)
  {
    return 0;
  }

  const char* analysisGridName = gridRepresentation->GetGridFileName();
  if (!analysisGridName || !(*analysisGridName))
  {
    vtkWarningMacro("This vtkModelGridRepresentation does not have a GridFileName.");
    return 0;
  }

  ostream& os = *(this->Stream);
  vtkIndent indent = parentIndent->GetNextIndent();

  // Open the primary element.
  os << indent << "<" << this->GetDataSetName();
  // version
  std::stringstream version;
  version << this->GetDataSetMajorVersion() << "." << this->GetDataSetMinorVersion();
  os << " version=\"" << version.str().c_str() << "\"";
  // analysis grid name
  os << " AnalysisMeshFileName=\""
     << vtksys::SystemTools::GetFilenameName(std::string(analysisGridName)).c_str() << "\"";
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

int vtkCMBMeshToModelWriter::WriteFooter(vtkIndent* parentIndent)
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

int vtkCMBMeshToModelWriter::Write3DModelMeshInfo(vtkIndent* parentindent)
{
  vtkDiscreteModel* Model = this->ModelWrapper->GetModel();
  vtkModelGridRepresentation* gridRepresentation = Model->GetAnalysisGridInfo();
  if (gridRepresentation == NULL)
  {
    return 0;
  }
  vtkModelBCGridRepresentation* analysisGridInfo =
    vtkModelBCGridRepresentation::SafeDownCast(gridRepresentation);
  if (!analysisGridInfo || analysisGridInfo->IsModelConsistent(Model) == false)
  {
    vtkWarningMacro("Only the vtkModelBCGridRepresentation, which is created from an "
      << "Omicron bc file, is supported currently for 3D model.");
    return 0;
  }

  vtkNew<vtkPolyData> tmpPoly;
  tmpPoly->Initialize();
  // the analysis grid info came from a bc file
  vtkIdList* idList = vtkIdList::New();
  // format is floating edge id, number of point ids, point ids,...
  if (Model->GetNumberOfAssociations(vtkModelEdgeType))
  {
    vtkIdTypeArray* floatingEdgeData = vtkIdTypeArray::New();
    vtkModelItemIterator* edges = Model->NewIterator(vtkModelEdgeType);
    vtkIdType floatingEdgeCounter = 0;
    for (edges->Begin(); !edges->IsAtEnd(); edges->Next())
    {
      vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
      if (edge->GetModelRegion())
      {
        floatingEdgeCounter++;
        floatingEdgeData->InsertNextValue(edge->GetUniquePersistentId());
        analysisGridInfo->GetFloatingEdgeAnalysisGridPointIds(
          Model, edge->GetUniquePersistentId(), idList);
        floatingEdgeData->InsertNextValue(idList->GetNumberOfIds());
        for (vtkIdType i = 0; i < idList->GetNumberOfIds(); i++)
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
  for (faces->Begin(); !faces->IsAtEnd(); faces->Next())
  {
    vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(faces->GetCurrentItem());
    modelFaceData->InsertNextValue(face->GetUniquePersistentId());
    analysisGridInfo->GetModelFaceAnalysisFacets(
      Model, face->GetUniquePersistentId(), idList, cellSides);
    modelFaceData->InsertNextValue(idList->GetNumberOfIds());
    for (vtkIdType i = 0; i < idList->GetNumberOfIds(); i++)
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
  if (this->DataMode == vtkXMLWriter::Appended)
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

int vtkCMBMeshToModelWriter::Write2DModelMeshInfo(vtkIndent* parentindent)
{
  vtkDiscreteModel* Model = this->ModelWrapper->GetModel();
  vtkModelGridRepresentation* gridRepresentation = Model->GetAnalysisGridInfo();
  if (gridRepresentation == NULL)
  {
    return 0;
  }
  vtkCMBMeshGridRepresentationServer* analysisGridInfo =
    vtkCMBMeshGridRepresentationServer::SafeDownCast(gridRepresentation);
  if (!analysisGridInfo || !analysisGridInfo->GetRepresentation() ||
    analysisGridInfo->IsModelConsistent(Model) == false)
  {
    vtkWarningMacro("Only the writing of vtkCMBMeshGridRepresentationServer "
      << "is supported currently for 2D model.");
    return 0;
  }
  vtkPolyData* gridPoly = analysisGridInfo->GetRepresentation();
  vtkIdTypeArray* ptsids = analysisGridInfo->GetPointIdMapArray();
  vtkIdTypeArray* cellids = analysisGridInfo->GetCellIdMapArray();
  if (!ptsids || !cellids)
  {
    vtkWarningMacro("One of the points or cell mapping arrays is missing.");
    return 0;
  }

  vtkIdType npts, *pts;
  vtkIdTypeArray* cellPointIds = NULL;
  vtkIdType numCells = gridPoly->GetNumberOfCells();
  if (numCells > 0)
  {
    cellPointIds = vtkIdTypeArray::New();
    cellPointIds->SetNumberOfComponents(3);
    cellPointIds->SetNumberOfTuples(numCells);
    cellPointIds->SetName(ModelFaceRep::Get2DAnalysisCellPointIdsString());
    vtkIdType* cpd = reinterpret_cast<vtkIdType*>(cellPointIds->GetVoidPointer(0));
    vtkCellArray* polys = gridPoly->GetPolys();
    polys->InitTraversal();
    while (polys->GetNextCell(npts, pts))
    {
      for (int j = 0; j < 3; j++)
      {
        *cpd = pts[j];
        cpd++;
      }
    }
  }
  else if (analysisGridInfo->GetCellPointIdsArray())
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
  if (numCells > 0 && cellPointIds)
  {
    cellPointIds->Delete();
  }
  vtkFieldData* fieldData = tmpPoly->GetFieldData();
  vtkIndent indent2 = parentindent->GetNextIndent();
  if (this->DataMode == vtkXMLWriter::Appended)
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

const char* vtkCMBMeshToModelWriter::GetDataSetName()
{
  return "PolyData";
}

void vtkCMBMeshToModelWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelWrapper: " << this->ModelWrapper << "\n";
}

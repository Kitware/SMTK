//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBTriangleMesher.h"

#include "vtkAppendPolyData.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkDataArray.h"
#include "vtkErrorCode.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolygon.h"
#include "vtkSmartPointer.h"
#include "vtkTimerLog.h"

#include <vtksys/SystemTools.hxx>

#include <algorithm>
#include <list>
#include <set>

#include "smtk/extension/vtk/meshing/cmbFaceMesherInterface.h"
#include "smtk/extension/vtk/meshing/cmbFaceMeshHelper.h"
#include "smtk/extension/vtk/meshing/vtkCMBMeshServerLauncher.h"
#include "smtk/extension/vtk/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "smtk/extension/vtk/meshing/vtkCMBUniquePointSet.h"

vtkCxxSetObjectMacro(vtkCMBTriangleMesher,Launcher,vtkCMBMeshServerLauncher);

//Unique Cell is used for appending multiple polydata's together that may
//Share cells. Storing all the cells in a set of UniqueCells gaurentees
//Uniqueness of the output
class UniqueCell
  {
  public:
    UniqueCell(const vtkIdType& npts, const vtkIdType* pts, const vtkIdType& inElementId)
      {
      this->nptIds = npts;
      this->elementId = inElementId;
      this->ptIds = new vtkIdType[this->nptIds];
      for(int i = 0; i < nptIds; i++)
        {
        this->ptIds[i] = pts[i];
        }
      std::sort(this->ptIds, this->ptIds+this->nptIds); //sort so the cell (2,1) and (1,2) are treated the same way
      }
    UniqueCell(const UniqueCell& other)
      {
      this->nptIds = other.nptIds;
      this->elementId = other.elementId;
      this->ptIds = new vtkIdType[this->nptIds];
      for(int i = 0; i < nptIds; i++)
        {
        this->ptIds[i] = other.ptIds[i];
        }
      }

    ~UniqueCell()
      {
      delete []ptIds;
      }

    bool operator < (const UniqueCell& c) const
      {
      //If they don't have the same number of points they can't be equal
      if(this->nptIds != c.nptIds){ return this->nptIds < c.nptIds; }
      for(int i = 0; i < nptIds; i++)
        {
        if(this->ptIds[i] < c.ptIds[i])
          {
          return true;
          }
        if(this->ptIds[i] > c.ptIds[i])
          {
          return false;
          }
        }
      return false;
      }

    vtkIdType* ptIds;
    vtkIdType nptIds;
    vtkIdType elementId;
  };

//Takes a list of polydatas as input and assumes they have
//a celldata array called elementIds (originally from a map file)
//outputs one polydata with no duplicate edges or points
//This will not work on nonpreserved edges the inputs must agree
void SmartMapAppend(std::list<vtkPolyData*> inputs, vtkPolyData* output,
                    bool PreserveEdgesAndNodes)
  {
  //if we have nothing to append to do anything at all
  if (inputs.size() == 0)
    {
    return;
    }

  // If there is 1 pd don't bother merging
  if (inputs.size() == 1)
    {
    output->ShallowCopy((*inputs.begin()));
    (*inputs.begin())->Delete();
    return;
    }

  //Create a unique set for points, verts, lines, and triangles
  vtkCMBUniquePointSet uniquePoints;
  std::set<UniqueCell> uniqueVertexes;
  std::set<UniqueCell> uniqueLines;
  std::set<UniqueCell> uniqueTriangles;

  //Iterate over the inputs and populate the unique sets
  std::list<vtkPolyData*>::const_iterator inputIter = inputs.begin();
  for(;inputIter != inputs.end(); inputIter++)
    {
    vtkPolyData* toMerge = (*inputIter);
    std::vector< vtkIdType > oldPtId2newPtId;
    oldPtId2newPtId.reserve(toMerge->GetPoints()->GetNumberOfPoints());

    //Populate uniquePoints
    for(int i = 0; i < toMerge->GetPoints()->GetNumberOfPoints(); i++)
      {
      double pt[3];
      toMerge->GetPoints()->GetPoint(i,pt);
      oldPtId2newPtId.push_back(uniquePoints.addPoint(pt));
      }

    //Populate the unique Cells
    vtkIdTypeArray *elementIds = vtkIdTypeArray::SafeDownCast(toMerge->GetCellData()->GetArray("ElementIds"));
    for(int i = 0; i < toMerge->GetNumberOfCells(); i++)
      {
      vtkCell* cell = toMerge->GetCell(i);
      vtkIdList* cellPointIds = cell->GetPointIds();
      vtkIdType elementId = PreserveEdgesAndNodes ? elementIds->GetTuple1(i) : -1;

      if(cell->GetCellType() == VTK_VERTEX)
        {
        vtkIdType newPtIds[1] = {oldPtId2newPtId[cellPointIds->GetId(0)]};
        uniqueVertexes.insert(UniqueCell(1,newPtIds,elementId));
        }
      if(cell->GetCellType() == VTK_LINE)
        {
        vtkIdType newPtIds[2] = {oldPtId2newPtId[cellPointIds->GetId(0)],oldPtId2newPtId[cellPointIds->GetId(1)]};
        uniqueLines.insert(UniqueCell(2,newPtIds,elementId));
        }
      if(cell->GetCellType() == VTK_TRIANGLE)
        {
        vtkIdType newPtIds[3] = {oldPtId2newPtId[cellPointIds->GetId(0)],oldPtId2newPtId[cellPointIds->GetId(1)],oldPtId2newPtId[cellPointIds->GetId(2)]};
        uniqueTriangles.insert(UniqueCell(3,newPtIds,elementId));
        }
      }
    toMerge->Delete(); //We are done with this polydata, delete it
    }

  //Allocate enough space for the output polydata
  output->Allocate(uniqueVertexes.size() + uniqueLines.size() + uniqueTriangles.size());
  //Allocate space for points
  vtkPoints* outputPts = vtkPoints::New();
  outputPts->SetNumberOfPoints(uniquePoints.getNumberOfPoints());
  //Allocate space for celldata if one originally existed
  vtkIdTypeArray* outputElementIds = NULL;
  if(PreserveEdgesAndNodes)
    {
    outputElementIds = vtkIdTypeArray::New();
    outputElementIds->SetNumberOfTuples(uniqueVertexes.size() + uniqueLines.size() + uniqueTriangles.size());
    outputElementIds->SetNumberOfComponents(1);
    outputElementIds->SetName("ElementIds");
    }

  //Iterate the unique points and add them to the polydata
  vtkIdType cellInsertAt = 0;
  for(int i = 0; i < uniquePoints.getNumberOfPoints(); i++)
    {
    double x=0,y=0;
    uniquePoints.getPoint(i,x,y);
    outputPts->SetPoint(i,x,y,0);
    }
  output->SetPoints(outputPts);
  outputPts->FastDelete();

  //Iterate over the unique cells and add them to the polydata
  std::set<UniqueCell>::const_iterator iter;
  for(iter = uniqueVertexes.begin(); iter != uniqueVertexes.end(); iter++)
    {
    vtkIdType ptsToInsert[1] = {iter->ptIds[0]};
    output->InsertNextCell(VTK_VERTEX,1,ptsToInsert);
    if(PreserveEdgesAndNodes)
      {
      outputElementIds->SetTuple1(cellInsertAt++,(*iter).elementId);
      }
    }
  for(iter = uniqueLines.begin(); iter != uniqueLines.end(); iter++)
    {
    vtkIdType ptsToInsert[2] = {iter->ptIds[0],iter->ptIds[1]};
    output->InsertNextCell(VTK_LINE,2,ptsToInsert);
    if(PreserveEdgesAndNodes)
      {
      outputElementIds->SetTuple1(cellInsertAt++,(*iter).elementId);
      }
    }
  for(iter = uniqueTriangles.begin(); iter != uniqueTriangles.end(); iter++)
    {
    vtkIdType ptsToInsert[3] = {iter->ptIds[0],iter->ptIds[1],iter->ptIds[2]};
    output->InsertNextCell(VTK_TRIANGLE,3,ptsToInsert);
    if(PreserveEdgesAndNodes)
      {
      outputElementIds->SetTuple1(cellInsertAt++,(*iter).elementId);
      }
    }

  //Add the element Ids
  if(PreserveEdgesAndNodes)
    {
    output->GetCellData()->AddArray(outputElementIds);
    outputElementIds->FastDelete();
    }
  }


vtkStandardNewMacro(vtkCMBTriangleMesher);

//--------------------------------------------------------------------
vtkCMBTriangleMesher::vtkCMBTriangleMesher()
{
  MinAngle              = 20.0f;
  UseMinAngle           = false;
  PreserveBoundaries    = true;
  PreserveEdgesAndNodes = false;
  MaxArea               = 1.0/8.0;
  ComputedMaxArea       = MaxArea;
  UseUniqueAreas        = false;
  MaxAreaMode           = RelativeToBoundsAndSegments;
  VerboseOutput         = false;
  Launcher              = NULL;
}
//--------------------------------------------------------------------
vtkCMBTriangleMesher::~vtkCMBTriangleMesher()
{
  this->SetLauncher(NULL);
}
//--------------------------------------------------------------------
void vtkCMBTriangleMesher::PrintSelf(ostream& os, vtkIndent indent)
{
  const char* areaModeType[4] =
    {
    "NoMaxArea",
    "AbsoluteArea",
    "RelativeToBounds",
    "RelativeToBoundsAndSegments"
    };
  os << indent << this->GetClassName() << endl;
  os << indent << "     Use Minimum Angle: " << UseMinAngle << endl;
  os << indent << "         Minimum Angle: " << MinAngle << endl;
  os << indent << "   Preserve Boundaries: " << PreserveBoundaries << endl;
  os << indent << "Preserve Edges & Nodes: " << PreserveEdgesAndNodes << endl;
  os << indent << "              Max Area: " << MaxArea << endl;
  os << indent << "     Computed Max Area: " << ComputedMaxArea << endl;
  os << indent << "         Max Area Mode: " << areaModeType[MaxAreaMode] << endl;
  os << indent << "      Use Unique Areas: " << UseUniqueAreas << endl;
  os << indent << "        Verbose Output: " << VerboseOutput << endl;
  os << indent << "  Mesh Server Launcher: " << Launcher << endl;
  this->Superclass::PrintSelf(os,indent);
}

using namespace CmbFaceMesherClasses;
using namespace vtksys;

//--------------------------------------------------------------------
int vtkCMBTriangleMesher::RequestData(vtkInformation * /*request*/,
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  //input is a vtkPolyData with field data specified by the
  //Map interface. This information is used for meshing
  vtkPolyData *input = vtkPolyData::GetData(inputVector[0]);
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  std::map<vtkIdType, ModelFaceRep* > pid2Face; // Create a face for every polygon

  { //scope this so we delete the class once the pid2Face has been filled
  vtkNew<vtkCMBPrepareForTriangleMesher> mapInterface;
  mapInterface->SetPolyData(input);
  bool valid = mapInterface->IsValidForReading();
  if(!valid)
    {
    vtkErrorMacro("Input poly data doesn't have the needed data for meshing ");
    vtkErrorMacro("Please use vtkCMBPrepareForTriangleMesher first.");
    return 1;
    }
  // Build the data structures required for meshing
  mapInterface->GetPolyId2ModelFaceRepMap(pid2Face);
  } //release mapInterface

  // Find global area information
  double* bnds = input->GetBounds();
  double totalPolyDataArea = (bnds[1]-bnds[0]) * (bnds[3]-bnds[2]);

  // Mesh each polygon individually then append all their polydata together
  std::list<vtkPolyData*> toAppend;
  std::map<vtkIdType, ModelFaceRep* >::iterator faceIter = pid2Face.begin();

  vtkCMBMeshServerLauncher* meshServer = this->GetLauncher();
  if (!meshServer)
    {
    vtkNew<vtkCMBMeshServerLauncher> launcher;
    meshServer = launcher.GetPointer();
    this->SetLauncher(meshServer);
    }
  for(; faceIter != pid2Face.end(); faceIter++)
    {
    vtkPolyData* outputMesh = vtkPolyData::New();
    vtkIdType faceId = (*faceIter).first;
    ModelFaceRep* face = (*faceIter).second;

    // Find local area information
    double currArea = totalPolyDataArea;
    double currNumSeg = input->GetNumberOfLines();

    if (UseUniqueAreas)
      {
      double faceBnds[4];
      face->bounds(faceBnds);
      currArea = (faceBnds[2]-faceBnds[0]) * (faceBnds[3]-faceBnds[1]);
      currNumSeg = face->numberOfEdges();
      }
    switch(MaxAreaMode)
      {
      case NoMaxArea:
        break;
      case AbsoluteArea:
        //If it is absolutly known what the max triangle size should
        //be just set it
        this->ComputedMaxArea = MaxArea;
        break;
      case RelativeToBounds:
        //Use the MaxArea as a ratio if areas are supposed to be
        //calculated relative to bounds
        this->ComputedMaxArea = currArea * MaxArea;
        break;
      case RelativeToBoundsAndSegments:
        // For added fidelity you can incorporate how coarse
        // or complicated a polygon is by dividing area by the
        // number of line segments in the polygon
        this->ComputedMaxArea = currArea / currNumSeg * MaxArea;
        break;
      default:
        vtkErrorMacro("ERROR: Invalid Max Area Mode");
        break;
      }

    cmbFaceMesherInterface ti(face->numberOfVertices(),
                              face->numberOfEdges(),
                              face->numberOfHoles(),
                              0,
                              this->PreserveEdgesAndNodes);
    ti.setUseMaxArea(this->MaxAreaMode != NoMaxArea);
    ti.setMaxArea(this->ComputedMaxArea);
    ti.setUseMinAngle(this->UseMinAngle);
    ti.setMinAngle(this->MinAngle);
    ti.setPreserveBoundaries(this->PreserveBoundaries);
    ti.setVerboseOutput(this->VerboseOutput);
    ti.setOutputMesh(outputMesh);
    face->fillTriangleInterface(&ti);
    bool faceBuilt = ti.buildFaceMesh(meshServer,faceId);
    if(faceBuilt)
      {
      toAppend.push_back(outputMesh);
      }
    }

  // Free memory associated with model faces in pid2Face
  faceIter = pid2Face.begin();
  for(; faceIter != pid2Face.end(); ++faceIter)
    {
    delete faceIter->second;
    }

  SmartMapAppend(toAppend,output, this->PreserveEdgesAndNodes);

  //Hack to get the output in map format
  //All the correct information is there, we just dont know where
  //the arc cells are, but that can be gotten by looking at the
  //elementIds
  if(this->PreserveEdgesAndNodes)
    {
    vtkFieldData* fieldData = input->GetFieldData();
    vtkIdTypeArray* fieldEndpoint1 = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("ArcEndpoint1"));
    vtkIdTypeArray* fieldEndpoint2 = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("ArcEndpoint2"));
    vtkIdTypeArray* fieldArcId     = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("ArcId"));
    vtkIdTypeArray* fieldLoop1 = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("Loop1"));
    vtkIdTypeArray* fieldLoop2 = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("Loop2"));
    vtkIdTypeArray* fieldLoopInfo  = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("LoopInfo"));

    output->GetFieldData()->AddArray(fieldEndpoint1);
    output->GetFieldData()->AddArray(fieldEndpoint2);
    output->GetFieldData()->AddArray(fieldArcId);
    output->GetFieldData()->AddArray(fieldLoop1);
    output->GetFieldData()->AddArray(fieldLoop2);
    output->GetFieldData()->AddArray(fieldLoopInfo);
    }
  return true;
}

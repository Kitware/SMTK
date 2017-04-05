//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME cmbFaceMesherInterface
// .SECTION Description
// Wraps the Triangle library with an easy to use class

#include "cmbFaceMesherInterface.h"

//needed to launch a cmb mesh server
#include "smtk/extension/vtk/meshing/vtkCMBMeshServerLauncher.h"

//needed to act like remus client
#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkNew.h>
#include <vtkPolyData.h>

#include <vector>

struct TriangleOutput
  {
  std::vector<double> points;
  std::vector<double> pointAttribute;

  std::vector<int> segments;
  std::vector<int> segmentMarker;

  std::vector<int> triangles;
  std::vector<double> triangleAttribute;
  };

template<typename T>
bool AllocFromStream(std::stringstream& buffer, std::vector<T> &dest, int numElements)
{
  if(numElements <= 0)
    {return true;}

  dest.resize(numElements);
  char* cdest = reinterpret_cast<char*>(&dest[0]);
  const std::streamsize size = sizeof(T)*numElements;

  //strip away the new line character at the start
  if(buffer.peek()=='\n')
    {buffer.get();}

  buffer.read(cdest,size);
  return buffer.gcount() == size;
}

template<typename T>
bool WriteToStream(std::stringstream& buffer, std::vector<T>& src, int numElements)
{
  if(numElements <= 0)
    {return true;}

  //now we fill it from the buffer
  char* csrc = reinterpret_cast<char*>(&src[0]);
  const std::streamsize size = sizeof(T)*numElements;
  buffer.write(csrc,size);
  buffer << std::endl;
  return !buffer.bad();
}


struct cmbFaceMesherInterface::TriangleInput
  {
  std::vector<double> points;
  std::vector<int> segments;
  std::vector<double> holes;

  std::vector<double> pointAttribute;
  std::vector<int> segmentMarker;
  std::vector<double> regions;

  };


cmbFaceMesherInterface::cmbFaceMesherInterface(const int &numPoints,
  const int &numSegments, const int &numHoles, const int& numRegions, const bool &preserveEdgesAndNodes):
  OutputMesh(NULL),
  MinAngleOn(false),
  MaxAreaOn(false),
  MaxArea(-1),
  MinAngle(-1),
  PreserveBoundaries(true),
  PreserveEdgesAndNodes(preserveEdgesAndNodes),
  VerboseOutput(false),
  NumberOfPoints(numPoints),
  NumberOfSegments(numSegments),
  NumberOfHoles(numHoles),
  NumberOfRegions(numRegions),
  NumberOfNodes(0),
  Ti(new cmbFaceMesherInterface::TriangleInput())
{
  this->InitDataStructures();
}

cmbFaceMesherInterface::~cmbFaceMesherInterface()
{
  delete this->Ti;
  this->OutputMesh = NULL;
}

void cmbFaceMesherInterface::InitDataStructures()
{
  this->Ti->points.resize(this->NumberOfPoints*2);
  this->Ti->segments.resize(this->NumberOfSegments*2);
  this->Ti->holes.resize(this->NumberOfHoles*2);

  this->Ti->regions.resize(this->NumberOfRegions*4);
  if (this->PreserveEdgesAndNodes)
    {
    this->Ti->segmentMarker.resize(this->NumberOfSegments);
    this->Ti->pointAttribute.resize(this->NumberOfPoints);

    }
}

void cmbFaceMesherInterface::setOutputMesh(vtkPolyData *mesh)
{
  this->OutputMesh = mesh;
}

bool cmbFaceMesherInterface::setPoint(const int index, const double &x, const double &y, const int& nodeId)
{
  if (index >= 0 && index < this->NumberOfPoints)
    {
    this->Ti->points[index*2]=x;
    this->Ti->points[index*2+1]=y;
    if(this->PreserveEdgesAndNodes)
      {
      if(nodeId != -1)
        {
        this->NumberOfNodes++;
        }
      this->Ti->pointAttribute[index] = nodeId;
      }
    return true;
    }
  return false;
}

bool cmbFaceMesherInterface::setSegment(const int index, const int &pId1, const int &pId2, const int &arcId)
{
  if (index >= 0 && index < this->NumberOfSegments)
    {
    this->Ti->segments[index*2]=pId1;
    this->Ti->segments[index*2+1]=pId2;
    if(this->PreserveEdgesAndNodes)
      {
      this->Ti->segmentMarker[index]=arcId;
      }
    return true;
    }
  return false;
}

bool cmbFaceMesherInterface::setRegion(const int index, const double &x, const double &y, const double &attribute, const double& max_area)
{
  if (index >= 0 && index < this->NumberOfRegions)
    {
    this->Ti->regions[index*4]=x;
    this->Ti->regions[index*4+1]=y;
    this->Ti->regions[index*4+2]=attribute;
    this->Ti->regions[index*4+3]=max_area;
    return true;
    }
  return false;
}

bool cmbFaceMesherInterface::setHole(const int index, const double &x, const double &y)
{
  if (index >= 0 && index < this->NumberOfHoles)
    {
    this->Ti->holes[index*2]=x;
    this->Ti->holes[index*2+1]=y;
    return true;
    }
  return false;
}

double cmbFaceMesherInterface::area() const
{
  double tmpBounds[4];
  this->bounds(tmpBounds);
  return (tmpBounds[2]-tmpBounds[0]) * (tmpBounds[3]-tmpBounds[1]);
}

void cmbFaceMesherInterface::bounds(double tmpBounds[4]) const
{
  if ( this->NumberOfPoints == 0 )
    {
    //handle the use case that we don't have any points yet
    tmpBounds[0]=tmpBounds[1]=tmpBounds[2]=tmpBounds[3]=0.0;
    return;
    }

  //use the first point as the min and max values
  int numPts = this->NumberOfPoints * 2;
  tmpBounds[0] = tmpBounds[2] = this->Ti->points[0];
  tmpBounds[1] = tmpBounds[3] = this->Ti->points[1];
  for ( int i=2; i < numPts; i+=2 )
    {
    tmpBounds[0] = this->Ti->points[i] < tmpBounds[0]?
      this->Ti->points[i] : tmpBounds[0];
    tmpBounds[2] = this->Ti->points[i] > tmpBounds[2]?
      this->Ti->points[i] : tmpBounds[2];
    tmpBounds[1] = this->Ti->points[i+1] < tmpBounds[1]?
      this->Ti->points[i+1] : tmpBounds[1];
    tmpBounds[3] = this->Ti->points[i+1] > tmpBounds[3]?
      this->Ti->points[i+1] : tmpBounds[3];
    }
}

bool cmbFaceMesherInterface::buildFaceMesh(const long &faceId,
                                           const double &zValue)
{
  vtkNew<vtkCMBMeshServerLauncher> serverLauncher;
  return this->buildFaceMesh(serverLauncher.GetPointer(),
                             faceId,
                             zValue);
}

bool cmbFaceMesherInterface::buildFaceMesh(vtkCMBMeshServerLauncher* activeServer,
                                           const long &faceId,
                                           const double &zValue)
{
  if(!activeServer->IsAlive())
    {
    return false;
    }

  bool valid = false;
  std::string hostName(activeServer->GetHostName());
  remus::client::ServerConnection conn(hostName,
                                       activeServer->GetPortNumber());


  remus::common::MeshIOType requestIOType( (remus::meshtypes::Edges()),
                                           (remus::meshtypes::Mesh2D()) );

  remus::proto::JobRequirements request =
                          remus::proto::make_JobRequirements( requestIOType,
                                                        "CMBMeshTriangleWorker",
                                                        "");
  remus::Client client(conn);
  if(client.canMesh(request))
    {

    //convert the data into a string
    std::string input_data;
    this->PackData(input_data);

    remus::proto::JobSubmission sub(request);
    sub["data"]=remus::proto::make_JobContent(input_data);
    remus::proto::Job job = client.submitJob(sub);
    remus::proto::JobStatus jobState = client.jobStatus(job);

    //wait while the job is running
    while(jobState.good())
      {
      jobState = client.jobStatus(job);
      };

    if(jobState.finished())
      {
      remus::proto::JobResult result = client.retrieveResults(job);
      // rip the data back into the data structures we expect.
      this->unPackData(result.data(),
                       result.dataSize(),
                       faceId,
                       zValue);
      valid = true;
      }
    else
      {
      valid = false;
      }
    }
  else
    {
    valid = false;
    }
  return valid;
}

bool cmbFaceMesherInterface::PackData(std::string& rawData)
{

  std::stringstream buffer;
  buffer << MinAngleOn << std::endl;
  buffer << MaxAreaOn << std::endl;
  buffer << PreserveBoundaries << std::endl;
  buffer << PreserveEdgesAndNodes << std::endl;
  buffer << NumberOfPoints << std::endl;
  buffer << NumberOfSegments << std::endl;
  buffer << NumberOfHoles << std::endl;
  buffer << NumberOfRegions << std::endl;
  buffer << NumberOfNodes << std::endl;
  buffer << MaxArea << std::endl;
  buffer << MinAngle << std::endl;

  WriteToStream(buffer,this->Ti->points, NumberOfPoints*2);
  WriteToStream(buffer,this->Ti->segments, NumberOfSegments*2);

  if(this->NumberOfHoles > 0)
    {
    WriteToStream(buffer,this->Ti->holes, NumberOfHoles*2);
    }

  if(this->NumberOfRegions > 0)
    {
    WriteToStream(buffer,this->Ti->regions, NumberOfRegions*4);
    }

  if (this->PreserveEdgesAndNodes)
    {
    WriteToStream(buffer,this->Ti->segmentMarker, NumberOfSegments);
    WriteToStream(buffer,this->Ti->pointAttribute, NumberOfPoints);
    }
  buffer << std::endl;
  rawData = buffer.str();
  return true;
}

bool cmbFaceMesherInterface::unPackData(const char* rawData,
                                        std::size_t dataSize,
                                        const long &faceId,
                                        const double &zValue)
{
  int numPoints, numTriangles, numLines;
  std::stringstream buffer( std::string(rawData, dataSize) );
  buffer >> numPoints;
  buffer >> numLines;
  buffer >> numTriangles;

  //we know have to convert the result into a vtkPolyData;
  if( numPoints == 0 || numTriangles == 0 || numLines == 0 )
    {
    vtkGenericWarningMacro("Failed to build a face mesh.");
    return false;
    }

  TriangleOutput out;
  AllocFromStream(buffer,out.points,numPoints*2);
  AllocFromStream(buffer,out.segments,numLines*2);
  AllocFromStream(buffer,out.triangles,numTriangles*3);

  if (this->PreserveEdgesAndNodes)
    {
    AllocFromStream(buffer,out.pointAttribute,numPoints);
    AllocFromStream(buffer,out.segmentMarker,numLines);
    }

  if (this->NumberOfRegions > 0)
    {
    AllocFromStream(buffer,out.triangleAttribute,numTriangles);
    }


  vtkIdTypeArray* elementIds = NULL;
  vtkIdType element_insert_loc = 0;
  //setup the nodes if requested
  if(this->PreserveEdgesAndNodes)
    {
    this->OutputMesh->Allocate(numTriangles+numLines+NumberOfNodes);
    elementIds = vtkIdTypeArray::New();
    elementIds->SetName("ElementIds");
    elementIds->SetNumberOfComponents(1);
    elementIds->SetNumberOfTuples(numTriangles+numLines+NumberOfNodes);
    }
  //setup the points
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(numPoints);
  for (vtkIdType i=0; i < numPoints; ++i)
    {
    points->InsertPoint(i,out.points[2*i],out.points[2*i+1],zValue);
    //Add a vertex cell data if we are preserving edges and nodes, there is a nonnegative id in the point attribute list
    //and we haven't gone over the number of nodes originally put in (If that check isn't there triangle likes to generate
    //steiner points with weird point attributes)
    if(this->PreserveEdgesAndNodes && out.pointAttribute[i] != -1 && element_insert_loc != this->NumberOfNodes)
      {
      this->OutputMesh->InsertNextCell(VTK_VERTEX,1,&i);
      elementIds->SetTuple1(element_insert_loc++,out.pointAttribute[i]);
      }
    }
  this->OutputMesh->SetPoints(points);
  points->FastDelete();

  //setup the lines if requested
  if(this->PreserveEdgesAndNodes)
    {
    vtkIdType line_ids[2];
    for(vtkIdType i=0; i < numLines; ++i)
      {
      line_ids[0] = out.segments[2 * i];
      line_ids[1] = out.segments[2 * i + 1];
      this->OutputMesh->InsertNextCell(VTK_LINE, 2, line_ids);
      elementIds->SetTuple1(element_insert_loc++,out.segmentMarker[i]);
      }
    }
  else
    {
    this->OutputMesh->Allocate(numTriangles);
    }

  //setup the triangles
  vtkIdType triangle_ids[3];
  for(vtkIdType i=0; i < numTriangles; ++i)
    {
    triangle_ids[0] =out.triangles[3 * i];
    triangle_ids[1] =out.triangles[3 * i + 1];
    triangle_ids[2] =out.triangles[3 * i + 2];

    assert(triangle_ids[0] < numPoints);
    assert(triangle_ids[1] < numPoints);
    assert(triangle_ids[2] < numPoints);

    this->OutputMesh->InsertNextCell(VTK_TRIANGLE, 3, triangle_ids);
    if(this->PreserveEdgesAndNodes && this->NumberOfRegions == 0)
      {
      //Element id for a triangle is the face id
      elementIds->SetTuple1(element_insert_loc++,faceId);
      }
    else if (this->NumberOfRegions > 0)
      {
      elementIds->SetTuple1(element_insert_loc++,out.triangleAttribute[i]);
      }
    }

  if(this->PreserveEdgesAndNodes)
    {
    this->OutputMesh->GetCellData()->AddArray(elementIds);
    elementIds->FastDelete();
    }

  return true;
}

// for Triangles
#undef ANSI_DECLARATORS
#undef VOID
#undef TRIANGLE_REAL

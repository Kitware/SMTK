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
// .NAME CmbFaceMesherInterface
// .SECTION Description
// Wraps the Triangle library with an easy to use class

#include "CmbFaceMesherInterface.h"

#include <sstream>

#include <vtkPolyData.h>

// for Triangle
#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#define VOID void
#endif

#ifndef TRIANGLE_REAL
#ifdef SINGLE
#define TRIANGLE_REAL float
#else                           /* not SINGLE */
#define TRIANGLE_REAL double
#endif
#endif

extern "C"
{
#include "triangle.h"
#include "share_declare.h"
  void Init_triangluateio(struct triangulateio *);
  void Free_triangluateio(struct triangulateio *);
  void triangle_report_vtk(char *filename, struct triangulateio *io);
}
// END for Triangle

struct CmbFaceMesherInterface::TriangleIO
  {
  triangulateio *in;
  triangulateio *out;
  triangulateio *vout;
  };

//----------------------------------------------------------------------------
CmbFaceMesherInterface::CmbFaceMesherInterface(const int &numPoints,
  const int &numSegments, const int &numHoles):
  MaxArea(-1),
  MinAngle(-1),
  MinAngleOn(false),
  MaxAreaOn(false),
  OutputMesh(NULL),
  TIO(new CmbFaceMesherInterface::TriangleIO()),
  NumberOfPoints(numPoints),
  NumberOfSegments(numSegments),
  NumberOfHoles(numHoles)
{
  this->InitDataStructures();
}
//----------------------------------------------------------------------------
CmbFaceMesherInterface::~CmbFaceMesherInterface()
{
  //there is a bug in triangle that the hole list is shared between
  //the in and out structs. So we have to set the hole list to NULL before
  //freeing the out, or the program will crash
  bool pointListShared = (this->TIO->in->pointlist == this->TIO->out->pointlist);
  bool segmentListShared = (this->TIO->in->segmentlist == this->TIO->out->segmentlist);
  bool holeListShared = (this->TIO->in->holelist == this->TIO->out->holelist);

  Free_triangluateio(this->TIO->in);
  if (pointListShared)
    {
    //The free on TIO->in released the memory
    this->TIO->out->pointlist=NULL;
    }
  if (segmentListShared)
    {
    //The free on TIO->in released the memory
    this->TIO->out->segmentlist=NULL;
    }
  if (holeListShared)
    {
    //The free on TIO->in released the memory
    this->TIO->out->holelist=NULL;
    }
  Free_triangluateio(this->TIO->out);
  this->OutputMesh = NULL;
}

//----------------------------------------------------------------------------
void CmbFaceMesherInterface::InitDataStructures()
{
  this->TIO->in = (triangulateio*)malloc(sizeof(triangulateio));
  Init_triangluateio(this->TIO->in);
  this->TIO->out = (triangulateio*)malloc(sizeof(triangulateio));
  Init_triangluateio(this->TIO->out);
  this->TIO->vout = NULL;

  //setup the memory for the trianle input
  this->TIO->in->numberofsegments = this->NumberOfSegments;
  this->TIO->in->numberofpoints = this->NumberOfPoints;
  this->TIO->in->numberofholes = this->NumberOfHoles;

  this->TIO->in->segmentlist =
  (int *) tl_alloc(sizeof(int),this->NumberOfSegments*2, 0);

  this->TIO->in->pointlist = (TRIANGLE_REAL *)
      tl_alloc(sizeof(TRIANGLE_REAL),this->NumberOfPoints * 2, 0);

  if (this->NumberOfHoles > 0)
    {
    this->TIO->in->holelist = (TRIANGLE_REAL *) tl_alloc(sizeof(TRIANGLE_REAL),
      this->NumberOfHoles * 2, 0);
    }
}

//----------------------------------------------------------------------------
void CmbFaceMesherInterface::setOutputMesh(vtkPolyData *mesh)
{
  this->OutputMesh = mesh;
}

//----------------------------------------------------------------------------
bool CmbFaceMesherInterface::setPoint(const int index, const double &x, const double &y)
{
  if (index >= 0 && index < this->NumberOfPoints)
    {
    this->TIO->in->pointlist[index*2]=x;
    this->TIO->in->pointlist[index*2+1]=y;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool CmbFaceMesherInterface::setSegement(const int index, const int &pId1, const int &pId2)
{
  if (index >= 0 && index < this->NumberOfSegments)
    {
    this->TIO->in->segmentlist[index*2]=pId1;
    this->TIO->in->segmentlist[index*2+1]=pId2;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool CmbFaceMesherInterface::setHole(const int index, const double &x, const double &y)
{
  if (index >= 0 && index < this->NumberOfHoles)
    {
    this->TIO->in->holelist[index*2]=x;
    this->TIO->in->holelist[index*2+1]=y;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
double CmbFaceMesherInterface::area() const
{
  double bounds[4];
  this->bounds(bounds);
  return (bounds[2]-bounds[0]) * (bounds[3]-bounds[1]);
}

//----------------------------------------------------------------------------
void CmbFaceMesherInterface::bounds(double bounds[4]) const
{
  if ( this->NumberOfPoints == 0 )
    {
    //handle the use case that we don't have any points yet
    bounds[0]=bounds[1]=bounds[2]=bounds[3]=0.0;
    return;
    }

  //use the first point as the min and max values
  int numPts = this->NumberOfPoints * 2;
  bounds[0] = bounds[2] = this->TIO->in->pointlist[0];
  bounds[1] = bounds[3] = this->TIO->in->pointlist[1];
  for ( int i=2; i < numPts; i+=2 )
    {
    bounds[0] = this->TIO->in->pointlist[i] < bounds[0]?
      this->TIO->in->pointlist[i] : bounds[0];
    bounds[2] = this->TIO->in->pointlist[i] > bounds[2]?
      this->TIO->in->pointlist[i] : bounds[2];
    bounds[1] = this->TIO->in->pointlist[i+1] < bounds[1]?
      this->TIO->in->pointlist[i+1] : bounds[1];
    bounds[3] = this->TIO->in->pointlist[i+1] > bounds[3]?
      this->TIO->in->pointlist[i+1] : bounds[3];
    }
}

//----------------------------------------------------------------------------
bool CmbFaceMesherInterface::BuildTriangleArguments(std::string &options) const
{
  bool valid = true;
  double value = 0;

  std::stringstream buffer;
  buffer << "p";//generate a planar straight line graph
  buffer << "z";//use 0 based indexing
  buffer << "Q";//enable quiet mode
  if(this->MaxAreaOn)
    {
    value = this->MaxArea;
    if (value < 0.0)
      {
      //invalid area constraint
      return false;
      }
    buffer << "a" << std::fixed << value;
    }
  if (this->MinAngleOn)
    {
    value = this->MinAngle;
    if (value < 0.0 || value > 33.)
      {
      vtkGenericWarningMacro("triangle mesher has min angle limit of 33.");
      //invalid area constraint
      return false;
      }
    buffer << "q" << std::fixed << value;
    }
  buffer << "Y";//preserve boundaries

  //assign
  options = buffer.str();
  return valid;
}

//----------------------------------------------------------------------------
bool CmbFaceMesherInterface::buildFaceMesh(const long &faceId)
{
  this->OutputMesh->Initialize();
  if ( this->NumberOfPoints < 3 || this->NumberOfSegments < 3 )
    {
    //passed an invalid data set
    return false;
    }

  //make sure the options string that is constructed is valid
  std::string options;
  bool validOptions = this->BuildTriangleArguments(options);
  if (!validOptions)
    {
    return false;
    }

  size_t len = options.size();
  char *switches = new char[len+1];
  strncpy(switches,options.c_str(),len);
  switches[len]='\0';

  int ret = triangulate(switches,this->TIO->in,this->TIO->out,this->TIO->vout);
  delete[] switches;

  if ( ret != 0 )
    {
    //triangle failed to mesh properly
    //C functions return not zero on failure
    return false;
    }

  //std::stringstream buffer;
  //buffer << "E:/Work/in" << faceId;
  //triangle_report_vtk(const_cast<char*>(buffer.str().c_str()),this->TIO->in);
  //buffer.str("");
  //buffer << "E:/Work/out" << faceId;
  //triangle_report_vtk(const_cast<char*>(buffer.str().c_str()),this->TIO->out);

  //we know have to convert the result into a vtkPolyData;
  triangulateio *io = this->TIO->out;
  if( io->numberofpoints == 0 ||
      io->numberoftriangles == 0 ||
      io->numberofsegments == 0 )
    {
    vtkGenericWarningMacro("Failed to build a face mesh.");
    return false;
    }

  //setup the points
  vtkPoints *points = vtkPoints::New();
  vtkIdType i=0;
  vtkIdType size = io->numberofpoints;
  points->SetNumberOfPoints(size);
  for (i=0; i < size; ++i)
    {
    points->InsertPoint(i,io->pointlist[2*i],io->pointlist[2*i+1],0.0);
    }
  this->OutputMesh->SetPoints(points);
  points->FastDelete();

  //setup the triangles
  size = io->numberoftriangles;
  this->OutputMesh->Allocate(size);
  vtkIdType ids[3];
  for(vtkIdType i=0; i < size; ++i)
    {
    ids[0] = io->trianglelist[3 * i];
    ids[1] = io->trianglelist[3 * i + 1];
    ids[2] = io->trianglelist[3 * i + 2];
    this->OutputMesh->InsertNextCell(VTK_TRIANGLE, 3, ids);
    }

  return true;
}

// for Triangles
#undef ANSI_DECLARATORS
#undef VOID
#undef TRIANGLE_REAL

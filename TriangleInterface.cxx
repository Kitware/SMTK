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
// .NAME TriangleInterface
// .SECTION Description
// Convert a vtkModelFace to a triangle input for meshing. Also
// restores the resulting mesh to a vtkPolyData

#include "TriangleInterface.h"

#include <vtkstd/map> // Needed for STL map.
#include <vtkstd/set> // Needed for STL set.
#include <vtkstd/list> // Needed for STL list.

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
#include "share_extern.h"
  void Init_triangluateio(struct triangulateio *);
  void Free_triangluateio(struct triangulateio *);
}
// END for Triangle

struct TriangleInterface::TriangleIO
  {
  triangulateio *in;
  triangulateio *out;
  triangulateio *vout;
  };

//----------------------------------------------------------------------------
TriangleInterface::TriangleInterface(const int &numPoints,
  const int &numSegments, const int &numHoles):
  MaxArea(-1),
  MinAngle(-1),
  MinAngleOn(false),
  MinAreaOn(false,
  OutputMesh(NULL),
  TIO(NULL),
  NumberOfPoints(numPoints);
  NumberOfSegments(numSegments);
  NumberOfHoles(numHoles);
{
  this->InitDataStructures(numPoints,numSegments,numHoles);
}

//----------------------------------------------------------------------------
void TriangleInterface::InitDataStructures()
{
  this->TIO->in = (triangulateio*)malloc(sizeof(triangulateio));
  Init_triangluateio(this->TIO->in);
  this->TIO->out = NULL;
  this->TIO->vout = NULL;

  //setup the memory for the trianle input
  this->TIO->in->numberofsegments = this->NumberOfSegments;
  this->TIO->in->numberofpoints = this->NumberOfPoints;
  this->TIO->in->numberofholes = this->NumberOfHoles;

  this->TIO->in->segmentlist =
  (int *) tl_alloc(sizeof(int),this->NumberOfSegments*2, 0);

  this->TIO->in->pointlist = (TRIANGLE_REAL *)
      tl_alloc(sizeof(TRIANGLE_REAL),this->NumberOfPoints * 2, 0);

  if (numHoles > 0)
    {
    this->TIO->in->holelist = (TRIANGLE_REAL *) tl_alloc(sizeof(TRIANGLE_REAL),
      this->NumberOfHoles * 2, 0);
    }
}

//----------------------------------------------------------------------------
void TriangleInterface::setOutputMesh(vtkPolyData *mesh)
{
  if ( this->OutputMesh )
    {
    this->OutputMesh->Delete();
    }
  this->OutputMesh->ShallowCopy(mesh);
}

//----------------------------------------------------------------------------
bool TriangleInterface::setPoint(const int index, const double &x, const double &y)
{
  if (index > 0 && index < this->NumberOfPoints)
    {
    this->TIO->in->pointlist[index*2]=x;
    this->TIO->in->pointlist[index*2+1]=y;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void TriangleInterface::setSegement(const int index, const int &pId1, const int &pId2)
{
  if (index > 0 && index < this->NumberOfSegments)
    {
    this->TIO->in->segmentlist[index*2]=pId1;
    this->TIO->in->segmentlist[index*2+1]=pId2;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void TriangleInterface::setHole(const int index, const double &x, const double &y)
{
  if (index > 0 && index < this->NumberOfHoles)
    {
    this->TIO->in->holelist[index*2]=x;
    this->TIO->in->holelist[index*2+1]=y;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
std::string TriangleInterface::BuildTriangleArguments() const
{
  std::stringstream buffer;
  buffer << "Q";//enable quiet mode
  buffer << "p";//generate a planar straight line graph
  buffer << "z";//use 0 based indexing
  buffer << "Y";//preserve boundaries
  if(this->MaxAreaOn)
    {
    buffer << "a" << fixed << this->MaxArea;
    }
  if (this->MinAngleOn)
    {
    buffer << "q" << fixed << this->MinAngle;
    }
  return buffer.str();
}

//----------------------------------------------------------------------------
bool TriangleInterface::buildFaceMesh()
{
  std::string options = this->BuildTriangleArguments();
  size_t len = options.size();
  char *switches = new char[len+1];
  strncpy(switches,options.c_str(),len);
  switches[len]='\0';

  triangulate(switches,this->TIO->in,this->TIO->out,this->TIO->vout);


  delete[] switches;
  return true;
}

// for Triangle
#undef ANSI_DECLARATORS
#undef VOID
#undef TRIANGLE_REAL
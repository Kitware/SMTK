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
// .NAME CmbFaceMeshHelper
// .SECTION Description
// Convert a vtkModelFace to a triangle input for meshing. Also
// restores the resulting mesh to a vtkPolyData



#ifndef __CmbFaceMeshHelper_h
#define __CmbFaceMeshHelper_h

#include <vtkstd/map> // Needed for STL map.
#include <vtkstd/set> // Needed for STL set.
#include <vtkstd/list> // Needed for STL list.
#include "vtkType.h"

//-----------------------------------------------------------------------------
namespace CmbModelFaceMeshPrivate
{
class InternalEdge
{
public:
  InternalEdge(const int &id, const int &edgeUse):
    Id(id), EdgeUse(edgeUse), numMeshPoints(0){}

  void addModelVert(const vtkIdType &id);
  void setNumberMeshPoints( const int &numPoints);

  //Note: verts and mesh points need to be added
  //before valid result is returned
  int numberLineSegments() const;

  int numberMeshPoints() const {return numMeshPoints;}

  std::set<vtkIdType>& modelVerts(){return ModelVerts;} const

  vtkIdType getId() const{return Id;}
  int getEdgeUse() const{return EdgeUse;}
protected:
  const vtkIdType Id;
  const int EdgeUse;
  int numMeshPoints;
  std::set<vtkIdType> ModelVerts;
};

class InternalLoop
{
public:
  InternalLoop(const vtkIdType &id):Hole(false),Id(id){}

  bool edgeExists(const vtkIdType &e) const;

  //only adds unique edges
  void addEdge(InternalEdge &edge);

  //returns the number of  unique points in this loop
  int getNumberOfPoints();

  //get the number of line segments in the loop
  int getNumberOfLineSegments() const;

  //returns if this loop is a hole
  bool isHole() const{return Hole;}
protected:
  //hole is recomputed every time an edge is added
  //if all the edges have an edge use > 1 than we are not a hole
  const vtkIdType Id;
  bool Hole;
  std::map<vtkIdType,InternalEdge> ModelEdges;
};

class MeshInformation
{
  public:
    void addLoop(const InternalLoop &loop);
    int numberOfPoints();
    int numberOfLineSegments();
    int numberOfHoles();

  protected:
    std::list<InternalLoop> Loops;
};
}
#endif
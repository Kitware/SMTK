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
// Convert a vtkModelFace to a triangle input for meshing.

#ifndef __CmbFaceMeshHelper_h
#define __CmbFaceMeshHelper_h

#include <vtkstd/map> // Needed for STL map.
#include <vtkstd/set> // Needed for STL set.
#include <vtkstd/vector> // Needed for STL vector.
#include "vtkType.h"

class CmbTriangleInterface;

//-----------------------------------------------------------------------------
namespace CmbModelFaceMeshPrivate
{
struct edgePoint
{
  double x, y;
  edgePoint(double a, double b):x(a),y(b){}
  bool operator <(const edgePoint &p) const {
    return x < p.x || (x == p.x && y < p.y);
  }
};

typedef vtkstd::pair<vtkIdType,vtkIdType> edgeSegment;
class InternalEdge
{
public:
  InternalEdge(const int &id, const int &edgeUse):
    Id(id), EdgeUse(edgeUse), numMeshPoints(0){}

  void addModelVert(const vtkIdType &id);
  void setMeshPoints(vtkPolyData *mesh);

  //Note: verts and mesh points need to be added
  //before valid result is returned
  int numberLineSegments() const;

  int numberMeshPoints() const {return MeshPoints.size();}

  std::set<vtkIdType> modelVerts(){return ModelVerts;} const
  std::vector<edgeSegment>& segments(){return Segments;} const
  std::map<vtkIdType,edgePoint>& meshPoints(){return MeshPoints;} const;

  vtkIdType getId() const{return Id;}
  int getEdgeUse() const{return EdgeUse;}
protected:
  const vtkIdType Id;
  const int EdgeUse;
  std::vector<edgeSegment> Segments;
  std::map<vtkIdType,edgePoint> MeshPoints;
  std::set<vtkIdType> ModelVerts;
};

class InternalLoop
{
public:
  InternalLoop(const vtkIdType &id):Hole(false),Id(id){}

  bool edgeExists(const vtkIdType &e) const;

  //only adds unique edges
  void addEdge(const InternalEdge &edge);

  //returns the number of  unique points in this loop
  int getNumberOfPoints();

  //get the number of line segments in the loop
  int getNumberOfLineSegments() const;

  //returns if this loop is a hole
  bool isHole() const{return Hole;}

  // adds this loops information to the triangle interface
  // modifies the pointIndex, segment Index, and HoleIndex
  void addDataToTriangleInterface(CmbTriangleInterface *ti,
     int &pointIndex, int &segmentIndex, int &holeIndex);

protected:
  //copy the information from the edge into the loop
  void addEdgeToLoop(const InternalEdge &edge);

  //Inserts the point if it doesn't exist, and returns
  //the vtkIdType id of the point.
  vtkIdType insertPoint(const edgePoint &point,
                        const vtkIdType &id);

  //hole is recomputed every time an edge is added
  //if all the edges have an edge use > 1 than we are not a hole
  const vtkIdType Id;
  bool Hole;
  std::set<vtkIdType> ModelEdges;

  std::vector<edgeSegment> Segments;
  std::map<edgePoint,vtkIdType> Points;
  std::set<vtkIdType> ModelVerts;
};

class MeshInformation
{
  public:
    void addLoop(const InternalLoop &loop);
    int numberOfPoints();
    int numberOfLineSegments();
    int numberOfHoles();

    fillTriangleInterface(CmbTriangleInterface *ti);

  protected:
    std::vector<InternalLoop> Loops;
};
}
#endif

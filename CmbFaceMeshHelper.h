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
#include <vtkstd/list> // Needed for STL list.
#include "vtkType.h"

class CmbTriangleInterface;
class vtkPolyData;

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
  InternalEdge(const int &id):Id(id){}

  void addModelVert(const vtkIdType &id);
  void setMeshPoints(vtkPolyData *mesh);

  //Note: verts and mesh points need to be added
  //before valid result is returned
  int numberLineSegments() const;

  int numberMeshPoints() const {return (int)MeshPoints.size();}

  std::set<vtkIdType> getModelVerts() const {return ModelVerts;}
  std::list<edgeSegment> getSegments() const {return Segments;}
  std::map<vtkIdType,edgePoint> getMeshPoints() const {return MeshPoints;}

  vtkIdType getId() const{return Id;}
protected:
  const vtkIdType Id;
  std::list<edgeSegment> Segments;
  std::map<vtkIdType,edgePoint> MeshPoints;
  std::set<vtkIdType> ModelVerts;
};

class InternalLoop
{
public:
  InternalLoop(const vtkIdType &id, const bool &isInternal)
    :EdgeCount(0),CanBeHole(isInternal),Id(id){}

  bool edgeExists(const vtkIdType &e) const;

  //only adds unique edges
  void addEdge(const InternalEdge &edge);

  //mark that we have a duplicate edge
  //this determines if the loop is a hole
  void markEdgeAsDuplicate(const vtkIdType &edgeId);

  //returns the number of  unique points in this loop
  int getNumberOfPoints();

  //get the number of line segments in the loop
  int getNumberOfLineSegments() const;

  //returns if this loop is a hole
  bool isHole() const;

  //returns NULL if point is not found
  const edgePoint* getPoint(const vtkIdType &id) const;

  // adds this loops information to the triangle interface
  // modifies the pointIndex, segment Index, and HoleIndex
  void addDataToTriangleInterface(CmbTriangleInterface *ti,
     int &pointIndex, int &segmentIndex, int &holeIndex);

  bool pointOnBoundary(const edgePoint &point) const;

  bool pointInside(const edgePoint &point) const;

  void bounds(double bounds[4]) const;

protected:
  //copy the information from the edge into the loop
  void addEdgeToLoop(const InternalEdge &edge);

  //Inserts the point if it doesn't exist, and returns
  //the vtkIdType id of the point.
  vtkIdType insertPoint(const edgePoint &point);

  const vtkIdType Id;
  const bool CanBeHole;

  //holds the number of unique edges we have. If greater than zero
  //we have a hole
  int EdgeCount;


  //these store ids, so we don't have duplicates
  std::set<vtkIdType> ModelEdges;
  std::set<vtkIdType> ModelVerts;

  std::list<edgeSegment> Segments;

  //bi directional map implemented as two maps
  std::map<edgePoint,vtkIdType> PointsToIds; //needed for easy lookup on duplicate points
  std::map<vtkIdType,edgePoint> IdsToPoints;
};

class MeshInformation
{
  public:
    void addLoop(const InternalLoop &loop);
    int numberOfPoints();
    int numberOfLineSegments();
    int numberOfHoles();

    void fillTriangleInterface(CmbTriangleInterface *ti);

  protected:
    std::list<InternalLoop> Loops;
};
}
#endif

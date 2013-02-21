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
// .NAME DiscreteMesh - CMB's implementation of a modifable mesh.
// .SECTION Description
// The implementation of a mesh that can represent 2D/3D or mixed type
// models.

#ifndef __DISCRETEMESH_H
#define __DISCRETEMESH_H

#include "vtkDiscreteModelModule.h" // For export macro

#include "vtkType.h" //needed for vtkIdType
#include "vtkSmartPointer.h" //needed for vtkSmartPointer
#include <vector> //needed for Face and FaceIds;

class vtkCell;
class vtkCellLocator;
class vtkIdList;
class vtkIncrementalOctreePointLocator;
class vtkPolyData;
class vtkPoints;

/*
 *## Friends that need to be rewritten ##
*/
//should use cell subset
class vtkDiscreteModelWrapper;
//it is too stateful. It is used in a tight loop to check points
//instead of given a collection of points to work on
class vtkEnclosingModelEntityOperator;
//what is the purpose of this class now that we have vtkDiscreteModelWrapper
class vtkCMBPolyDataProvider;
//we need to add a cell iterator for this class to be happy
class vtkCMBXMLBCSWriter;
/*
 *##  End of classes that need to be rewritten ##
*/


//currently this class is a shim to vtkPolyData, as we work
//on detaching vtkDiscreteModels, mesh representation for VTK
class VTKDISCRETEMODEL_EXPORT DiscreteMesh
{
public:
  typedef std::pair<vtkIdType,vtkIdType> EdgeResult;
  typedef std::vector<vtkIdType> FaceResult;

  class Edge;
  class Face;

  DiscreteMesh();
  DiscreteMesh(vtkPolyData* data);
  DiscreteMesh(const DiscreteMesh& other);
  DiscreteMesh& operator=(const DiscreteMesh&);

  virtual ~DiscreteMesh();

  bool IsValid() const;

  //verifies that Data is valid.
  vtkIdType GetNumberOfCells() const;

  //verifies thata Data is valid.
  vtkIdType GetNumberOfPoints() const;

  //verifies that Data is valid.
  //does a deep copy of Data into the returned polydata
  vtkSmartPointer<vtkPolyData> DeepCopy() const;

  //Doesn't verify Data is valid!
  //special method to signify that something is sharing
  //our points
  vtkPoints* SharePointsPtr() const;

  //Doesn't verify Data is valid!
  void BuildLinks() const;

  //Doesn't verify Data is valid!
  vtkSmartPointer<vtkIncrementalOctreePointLocator> BuildPointLocator() const;

  //Doesn't verify Data is valid!
  bool ComputeCellNormal(vtkIdType index, double norm[3]) const;

  //Doesn't verify Data is valid!
  bool ComputeCellCentroid(vtkIdType index, double centroid[3]) const;

  //Doesn't verify Data is valid!
  int GetCellType(vtkIdType index) const;

  //Doesn't verify Data is valid!
  void GetCellPointIds(vtkIdType index, vtkIdList* points) const;

  //Doesn't verify Data is valid!
  void GetCellNeighbors(vtkIdType index, vtkIdList* edge, vtkIdList* neighbors) const;

  void GetCellEdgeNeighbors(vtkIdType index, vtkIdType startEdge,
                            vtkIdType endEdge, vtkIdList* neighbors) const;

  //Doesn't verify Data is valid!
  //overwrite the points that represent the mesh
  void UpdatePoints(vtkPoints* points) const;

  //Doesn't verify Data is valid!
  void GetPoint(vtkIdType index, double xyz[3]) const;

  //Doesn't verify Data is valid!
  void GetPointCells(vtkIdType index, vtkIdList* cellsForPoint) const;

  //Doesn't verify Data is valid!
  void GetBounds(double bounds[6]) const;

  //Doesn't verify Data is valid!
  //Doesn't verify pos is out of bounds
  void MovePoint(vtkIdType pos, double xyz[3]) const;

  //Doesn't verify Data is valid!
  DiscreteMesh::EdgeResult AddEdge( const DiscreteMesh::Edge& e);

  DiscreteMesh::FaceResult AddFace( const DiscreteMesh::Face& f) const;

  friend class vtkDiscreteModelWrapper;
  friend class vtkEnclosingModelEntityOperator;
  friend class vtkCMBPolyDataProvider;
  friend class vtkCMBXMLBCSWriter;
private:
  //verifies that Data is valid.
  //does a shallow copy of Data into the returned polydata
  //private as people shouldn't break encapsulation
  vtkSmartPointer<vtkPolyData> ShallowCopy() const;


  vtkPolyData* Data;
};



//----------------------------------------------------------------------------
class DiscreteMesh::Edge : public std::pair<double[3],double[3]>
{
public:
  Edge(double f[3], double s[3])
  {
    this->first[0] = f[0];
    this->first[1] = f[1];
    this->first[2] = f[2];

    this->second[0] = s[0];
    this->second[1] = s[1];
    this->second[2] = s[2];
  }
};


//----------------------------------------------------------------------------
class DiscreteMesh::Face
{
  struct point
    {
    point(double p[3]):x(p[0]),y(p[1]),z(p[2]){}
    double x,y,z;
    };

  int CType;
  std::vector< vtkIdType > PointIds;
  std::vector< point > NewPoints;
public:
  typedef std::vector< vtkIdType >::const_iterator ids_const_iterator;
  typedef std::vector< point >::const_iterator points_const_iterator;

  Face(int cellType):
    CType(cellType),
    PointIds(),
    NewPoints()
  {
  }
  void AddExistingPointId(vtkIdType id)
    {
    this->PointIds.push_back(id);
    }

  void AddNewPoint(double pos[3])
    {
    this->NewPoints.push_back(Face::point(pos));
    this->PointIds.push_back(-1);
    }

  vtkIdType InvalidId() const { return -1; }

  int CellType() const { return this->CType; }

  ids_const_iterator ids_begin() const { return this->PointIds.begin(); }
  ids_const_iterator ids_end() const { return this->PointIds.end(); }

  points_const_iterator points_begin() const { return this->NewPoints.begin(); }
  points_const_iterator points_end() const { return this->NewPoints.end(); }

  vtkIdType GetNumberOfPoints() const { return this->PointIds.size(); }
};



#endif // __DISCRETEMESH_H

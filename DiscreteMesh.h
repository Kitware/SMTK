/*  =========================================================================

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

#include "DiscreteMeshCellIdIterator.h" //needed for iterator
#include "vtkType.h" //needed for vtkIdType
#include "vtkSmartPointer.h" //needed for vtkSmartPointer
#include <vector> //needed for Face and FaceIds;
#include <map> //needed for the edge storage

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

//we mangle the id spaces, so that all negative ids
//that are passed in represent edges, while all postive represent faces

class VTKDISCRETEMODEL_EXPORT DiscreteMesh
{
public:
  enum DataType{ FACE_DATA=0, EDGE_DATA=1, BOTH_DATA=2};

  typedef DiscreteMeshCellIdIterator cell_const_iterator;
  typedef std::pair<vtkIdType,vtkIdType> EdgePointIds;


  class EdgePoints;
  class Face;
  class FaceResult;

  DiscreteMesh();
  DiscreteMesh(vtkPolyData* allData);

  DiscreteMesh(const DiscreteMesh& other);
  DiscreteMesh& operator=(const DiscreteMesh&);

  virtual ~DiscreteMesh();

  bool IsValid() const;

  //verifies that Data is valid.
  vtkIdType GetNumberOfCells() const;

  //verifies that Data is valid.
  vtkIdType GetNumberOfFaces() const;

  //verifies that Data is valid.
  vtkIdType GetNumberOfEdges() const;

  //verifies that Data is valid.
  vtkIdType GetNumberOfPoints() const;

  cell_const_iterator CellsBegin() const;
  cell_const_iterator CellsEnd() const;
  //verifies that Data is valid.
  //does a deep copy of Edge and Face Data into the returned polydata
  vtkSmartPointer<vtkPolyData> GetAsSinglePolyData() const;

  //Doesn't verify Data is valid!
  //special method to signify that something is sharing
  //our points
  vtkPoints* SharePointsPtr() const;

  //Doesn't verify Data is valid!
  void BuildLinks() const;

  //Doesn't verify Data is valid!
  vtkSmartPointer<vtkIncrementalOctreePointLocator> BuildPointLocator( DataType type = FACE_DATA) const;

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

  //Get the neighbors at an edge.
  //More efficient than the general GetCellNeighbors().
  //Assumes links have been built (with BuildLinks()),
  //and looks specifically for edge neighbors.
  void GetCellEdgeNeighbors(vtkIdType cellIndex, vtkIdType pointIdOne,
                            vtkIdType pointIdTwo, vtkIdList* neighbors) const;

  //Doesn't verify Data is valid!
  //overwrite the points that represent the mesh
  void UpdatePoints(vtkPoints* points) const;

  //Doesn't verify Data is valid!
  void GetPoint(vtkIdType index, double xyz[3]) const;

  //Doesn't verify Data is valid!
  //Todo this need a modifier for edges
  void GetPointFaceCells(vtkIdType index, vtkIdList* cellsForPoint) const;

  //Doesn't verify Data is valid!
  void GetBounds(double bounds[6]) const;

  //Doesn't verify Data is valid!
  //Doesn't verify pos is out of bounds
  void MovePoint(vtkIdType pos, double xyz[3]) const;

  //Doesn't verify Data is valid!
  //doesn't add the edge to mesh, just adds the points to the shared points
  //array. EdgePoints is composed of two xyz double values
  DiscreteMesh::EdgePointIds AddEdgePoints(const DiscreteMesh::EdgePoints& e) const;


  //Returns true if the e or the inverse of the edge ( A,B or B,A ) exists
  //in the mesh. Will set edgeId with the meshIndex if the edge exists, otherwise
  //will not touch the parameter.
  bool EdgeExists(EdgePointIds& e, vtkIdType& edgeId) const;

  //Adds an edge to the mesh without any checks. Will return
  //the meshId for the edge. You can seriously break the mesh if you
  //add duplicate edges, or an existing edge in the opposite direction.
  //The edge is added in passed in order, so orientation is alway 1/true
  //THIS IS AN ADVANCED METHOD FOR OPTIMIZED ALGORITHMS
  vtkIdType AddEdge(EdgePointIds &e) const;

  //Conditional add an edge to the discrete mesh given two point ids.
  //If the edge already exists in the mesh this will return the existing
  //mesh id. It should be noted that the edge between A and B is considered
  //to be equal to the edge between B and A. This means that trying to
  //add the edge B,A when A,B exists will cause the function to return
  //the meshId for A,B
  vtkIdType AddEdgeIfNotExisting(EdgePointIds& e, bool& orientation)const;

  DiscreteMesh::FaceResult AddFace( const DiscreteMesh::Face& f) const;

  friend class vtkDiscreteModelWrapper;
  friend class vtkEnclosingModelEntityOperator;
  friend class vtkCMBPolyDataProvider;
  friend class vtkCMBXMLBCSWriter;
  friend class vtkDiscreteModelEdge;
private:
  //verifies that Data is valid.
  //does a shallow copy of Data into the returned polydata
  //private as people shouldn't break encapsulation
  vtkSmartPointer<vtkPolyData> ShallowCopyFaceData() const;

  //used by copy constructor and assign operator to simplify copying
  void ShallowAssignment(const DiscreteMesh& other);

  //given a data type ( face or edge ) return the corect poly data
  vtkPolyData* GetDataFromType(DiscreteMesh::DataType type ) const;

  //static method to help with conversion
  static inline void FlatIdSpaceToEdgeIdSpace(vtkIdType* cellIds, vtkIdType num)
    { for(vtkIdType i=0; i < num; ++i) { cellIds[i] ^= -1; } }

  vtkPolyData* FaceData;
  vtkPolyData* EdgeData;
  //both poly data's point to the same vtkPoints
  vtkPoints* SharedPoints;


};

//----------------------------------------------------------------------------
class DiscreteMesh::EdgePoints : public std::pair<double[3],double[3]>
{
public:
  EdgePoints(double f[3], double s[3])
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
  vtkIdType InvalidId() const { return -1; }

  void AddExistingPointId(vtkIdType id)
    {
    this->PointIds.push_back(id);
    }

  void AddNewPoint(double pos[3])
    {
    this->NewPoints.push_back(Face::point(pos));
    this->PointIds.push_back(InvalidId());
    }

  int CellType() const { return this->CType; }

  ids_const_iterator ids_begin() const { return this->PointIds.begin(); }
  ids_const_iterator ids_end() const { return this->PointIds.end(); }

  points_const_iterator points_begin() const { return this->NewPoints.begin(); }
  points_const_iterator points_end() const { return this->NewPoints.end(); }

  vtkIdType GetNumberOfPoints() const { return this->PointIds.size(); }
};

//----------------------------------------------------------------------------
class DiscreteMesh::FaceResult: private std::vector<vtkIdType>
{
  typedef vtkIdType T;
  typedef std::vector<vtkIdType> vector;
public:
  typedef vector::const_iterator const_iterator;
  typedef vector::iterator iterator;

  FaceResult():vector(),CellId(0){}

  using vector::front;
  using vector::begin;
  using vector::end;
  using vector::push_back;
  using vector::reserve;
  using vector::resize;
  using vector::size;
  using vector::operator[];
  vtkIdType CellId;

};



#endif // __DISCRETEMESH_H

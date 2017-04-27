//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_CellTraits_h
#define __smtk_mesh_CellTraits_h

#include "smtk/mesh/CellTypes.h"

namespace smtk
{
namespace mesh
{

template <int dimension>
struct CellTopologicalDimensionsTag
{
};

//these need to be comparable. so we use an integer type
struct CellFixedPointNumberTag
{
  const static int Type = 1;
}; //e.g line,triangle,hex
struct CellVariablePointNumberTag
{
  const static int Type = 2;
}; //e.g. polygon

/// The templated CellTraits struct provides the basic high level information
/// about cells such as:
///
/// TopologicalDimensionsTag: Defines the topological dimensions of the
//  cell type. 3 for polyhedra, 2 for polygons, 1 for lines, 0 for points.
//
// PointNumberTag: Defines if the number of points for this cell type
// is fixed at compile time or runtime
//
// Use bool fixedPointSize() const to return if the cell of this type
// has a fixed number of points at runtime
//
// Use int dimension() const to return the dimensionality of this cell type
// at runtime
//
template <class CellTag>
struct CellTraits;

template <>
struct CellTraits<smtk::mesh::CellHexahedron>
{
  const static int NUM_VERTICES = 8;
  const static int TOPOLOGICAL_DIMENSIONS = 3;

  typedef smtk::mesh::CellHexahedron CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<3> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellLine>
{
  const static int NUM_VERTICES = 2;
  const static int TOPOLOGICAL_DIMENSIONS = 1;

  typedef smtk::mesh::CellLine CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<1> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellPolygon>
{
  const static int NUM_VERTICES = -1;
  const static int TOPOLOGICAL_DIMENSIONS = 2;

  typedef smtk::mesh::CellPolygon CellType;
  typedef smtk::mesh::CellVariablePointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<2> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return false; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellPyramid>
{
  const static int NUM_VERTICES = 5;
  const static int TOPOLOGICAL_DIMENSIONS = 3;

  typedef smtk::mesh::CellPyramid CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<4> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellQuad>
{
  const static int NUM_VERTICES = 4;
  const static int TOPOLOGICAL_DIMENSIONS = 2;

  typedef smtk::mesh::CellQuad CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<2> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellTetrahedron>
{
  const static int NUM_VERTICES = 4;
  const static int TOPOLOGICAL_DIMENSIONS = 3;

  typedef smtk::mesh::CellTetrahedron CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<3> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellTriangle>
{
  const static int NUM_VERTICES = 3;
  const static int TOPOLOGICAL_DIMENSIONS = 2;

  typedef smtk::mesh::CellTriangle CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<2> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellVertex>
{
  const static int NUM_VERTICES = 1;
  const static int TOPOLOGICAL_DIMENSIONS = 0;

  typedef smtk::mesh::CellVertex CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<0> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <>
struct CellTraits<smtk::mesh::CellWedge>
{
  const static int NUM_VERTICES = 6;
  const static int TOPOLOGICAL_DIMENSIONS = 3;

  typedef smtk::mesh::CellWedge CellType;
  typedef smtk::mesh::CellFixedPointNumberTag PointNumberTag;
  typedef smtk::mesh::CellTopologicalDimensionsTag<3> TopologicalDimensionsTag;

  //return if this cell type has a compile time fixed number of cells
  bool fixedPointSize() const { return true; }

  //return the dimensionality of this cell type
  int dimension() const { return TOPOLOGICAL_DIMENSIONS; }
};

template <int CellEnum>
struct CellEnumToType;

template <>
struct CellEnumToType<smtk::mesh::Hexahedron>
{
  typedef smtk::mesh::CellHexahedron CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Line>
{
  typedef smtk::mesh::CellLine CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Polygon>
{
  typedef smtk::mesh::CellPolygon CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Pyramid>
{
  typedef smtk::mesh::CellPyramid CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Quad>
{
  typedef smtk::mesh::CellQuad CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Tetrahedron>
{
  typedef smtk::mesh::CellTetrahedron CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Triangle>
{
  typedef smtk::mesh::CellTriangle CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Vertex>
{
  typedef smtk::mesh::CellVertex CellType;
  typedef CellTraits<CellType> Traits;
};

template <>
struct CellEnumToType<smtk::mesh::Wedge>
{
  typedef smtk::mesh::CellWedge CellType;
  typedef CellTraits<CellType> Traits;
};

#define smtkMeshCellEnumToTypeMacroCase(cellEnum, call)                                            \
  case cellEnum:                                                                                   \
  {                                                                                                \
    static const int CellEnumValue = cellEnum;                                                     \
    typedef smtk::mesh::CellEnumToType<CellEnumValue>::Traits CellTraits;                          \
    call;                                                                                          \
  }                                                                                                \
  break;

#define smtkMeshCellEnumToTypeMacro(call)                                                          \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Vertex, call);                                       \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Line, call);                                         \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Triangle, call);                                     \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Quad, call);                                         \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Polygon, call);                                      \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Tetrahedron, call);                                  \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Pyramid, call);                                      \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Wedge, call);                                        \
  smtkMeshCellEnumToTypeMacroCase(smtk::mesh::Hexahedron, call);
}
}

#endif //__smtk_mesh_CellTraits_h

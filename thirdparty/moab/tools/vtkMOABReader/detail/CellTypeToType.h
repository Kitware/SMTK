#ifndef __smoab_detail_CellTypeToType_h
#define __smoab_detail_CellTypeToType_h

#include "vtkCellType.h"
#include <algorithm>

namespace smoab{ namespace detail{

template<int N> struct QuadratricOrdering{};

template<> struct QuadratricOrdering<VTK_QUADRATIC_WEDGE>
{
  static const int NUM_VERTS = 15;
  void reorder(vtkIdType* connectivity) const
  {
    std::swap_ranges(connectivity+9,connectivity+12,connectivity+12);
  }
};

template<> struct QuadratricOrdering<VTK_TRIQUADRATIC_HEXAHEDRON>
{
  static const int NUM_VERTS = 27;
  void reorder(vtkIdType* connectivity) const
  {
    std::swap_ranges(connectivity+12,connectivity+16,connectivity+16);

    //move 20 to 22
    //move 22 to 23
    //move 23 to 20

    //swap 20 with 22
    std::swap(connectivity[20],connectivity[23]);

    //swap 22 with 23
    std::swap(connectivity[22],connectivity[23]);
  }
};

template<typename QuadraticOrdering>
void FixQuadraticIdOrdering(vtkIdType* connectivity, vtkIdType numCells,
                            QuadraticOrdering& ordering)
{
  //skip the first index that holds the length of the cells
  //if we skip it once here, and than properly increment it makes the code
  //far easier
  connectivity+=1;
  for(vtkIdType i=0; i < numCells; ++i)
    {
    ordering.reorder(connectivity);
    connectivity += ordering.NUM_VERTS + 1;
    }
}


int vtkCellType(moab::EntityType t, int &num_connect)
  {
  int ctype = -1;
  switch (t)
    {
    case moab::MBEDGE:
      if (num_connect == 2) ctype = VTK_LINE;
      else if (num_connect == 3) ctype = VTK_QUADRATIC_EDGE;
      break;
    case moab::MBTRI:
      if (num_connect == 3) ctype = VTK_TRIANGLE;
      else if (num_connect == 6) ctype = VTK_QUADRATIC_TRIANGLE;
      else if (num_connect == 7) ctype = VTK_BIQUADRATIC_TRIANGLE;
      break;
    case moab::MBQUAD:
      if (num_connect == 4) ctype = VTK_QUAD;
      else if (num_connect == 8) ctype = VTK_QUADRATIC_QUAD;
      else if (num_connect == 9) ctype = VTK_BIQUADRATIC_QUAD;
      break;
    case moab::MBPOLYGON:
      if (num_connect == 4) ctype = VTK_POLYGON;
      break;
    case moab::MBTET:
      if (num_connect == 4) ctype = VTK_TETRA;
      else if (num_connect == 10) ctype = VTK_QUADRATIC_TETRA;
      break;
    case moab::MBPYRAMID:
      if (num_connect == 5) ctype = VTK_PYRAMID;
      else if (num_connect == 13) ctype = VTK_QUADRATIC_PYRAMID;
      break;
    case moab::MBPRISM:
      if (num_connect == 6) ctype = VTK_WEDGE;
      else if (num_connect == 15) ctype = VTK_QUADRATIC_WEDGE;
      break;
    case moab::MBHEX:
      if (num_connect == 8) ctype = VTK_HEXAHEDRON;
      else if (num_connect == 20) ctype = VTK_QUADRATIC_HEXAHEDRON;
      else if (num_connect == 21) ctype = VTK_QUADRATIC_HEXAHEDRON, num_connect = 20;
      else if (num_connect == 27) ctype = VTK_TRIQUADRATIC_HEXAHEDRON;
      break;
    default:
      ctype = -1;
      break;
    }
  return ctype;
  }

int vtkLinearCellType(moab::EntityType t, int &num_connect)
  {
  int ctype = -1;
  switch (t)
    {
    case moab::MBEDGE:
      ctype = VTK_LINE;
      num_connect = 2;
      break;
    case moab::MBTRI:
      ctype = VTK_TRIANGLE;
      num_connect = 3;
      break;
    case moab::MBQUAD:
      ctype = VTK_QUAD;
      num_connect = 4;
      break;
    case moab::MBPOLYGON:
      ctype = VTK_POLYGON;
      num_connect = 4;
      break;
    case moab::MBTET:
      ctype = VTK_TETRA;
      num_connect = 4;
      break;
    case moab::MBPYRAMID:
      ctype = VTK_PYRAMID;
      num_connect = 5;
      break;
    case moab::MBPRISM:
      ctype = VTK_WEDGE;
      num_connect = 6;
      break;
    case moab::MBHEX:
      ctype = VTK_HEXAHEDRON;
      num_connect = 8;
      break;
    default:
      break;
    }
  return ctype;
  }

} } //namespace smaob::detail

#endif // CELLTYPETOTYPE_H

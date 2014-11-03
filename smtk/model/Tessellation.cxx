//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

Tessellation::Tessellation()
{
}

int Tessellation::addCoords(double* a)
{
  std::vector<double>::size_type ipt = this->m_coords.size();
  for (int i = 0; i < 3; ++i)
    {
    this->m_coords.push_back(a[i]);
    }
  return static_cast<int>(ipt / 3);
}

Tessellation& Tessellation::addCoords(double x, double y, double z)
{
  this->m_coords.push_back(x);
  this->m_coords.push_back(y);
  this->m_coords.push_back(z);
  return *this;
}

Tessellation& Tessellation::addPoint(double* a)
{
  int ai = this->addCoords(a);
  (void)ai;
  std::vector<int> pconn;
  pconn.push_back(512); // Extension of three.js file format for "Vertex" glyph
  pconn.push_back(ai);
  this->m_conn.push_back(ai);
  return *this;
}

Tessellation& Tessellation::addLine(double* a, double* b)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  this->m_conn.push_back(2);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::addTriangle(double* a, double* b, double* c)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  int ci = this->addCoords(c);
  this->m_conn.push_back(0); // A triangle in three.js format.
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  this->m_conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::addPoint(int ai)
{
  this->m_conn.push_back(512);
  this->m_conn.push_back(ai);
  return *this;
}

Tessellation& Tessellation::addLine(int ai, int bi)
{
  this->m_conn.push_back(2);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::addTriangle(int ai, int bi, int ci)
{
  this->m_conn.push_back(0);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  this->m_conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::reset()
{
  this->m_conn.clear();
  this->m_coords.clear();
  return *this;
}

Tessellation::size_type Tessellation::begin() const
{
  return this->m_conn.empty() ? this->end() : 0;
}

Tessellation::size_type Tessellation::end() const
{ return -1; }

Tessellation::size_type Tessellation::nextCellOffset(size_type curOffset) const
{
  size_type cell_type;
  size_type num_verts = this->numberOfCellVertices(curOffset, &cell_type);
  if (num_verts == 0) return this->end();

  int num_cell_props = this->numCellPropsFromType(cell_type);
  int num_vert_props = this->numVertexPropsFromType(cell_type);

  size_type unchecked_next =
    curOffset + (cell_type & TESS_VARYING_VERT_CELL ? 2 : 1) + num_verts * (1 + num_vert_props) + num_cell_props;
  size_type next =
    (unchecked_next < 0 || unchecked_next >= static_cast<int>(this->m_conn.size())) ?
    this->end() :
    unchecked_next;
  return next;
}

Tessellation::size_type Tessellation::cellType(size_type offset) const
{
  return
    (offset < 0 || offset >= static_cast<int>(this->m_conn.size())) ?
    TESS_INVALID_CELL :
    this->m_conn[offset];
}

Tessellation::size_type Tessellation::numberOfCellVertices(
  size_type offset, size_type* cellTypeOut) const
{
  size_type cell_type = this->cellType(offset);
  if (cellTypeOut) *cellTypeOut = cell_type;

  size_type cell_shape = this->cellShapeFromType(cell_type);
  switch (cell_shape)
    {
  case TESS_VERTEX:           return 1;
  case TESS_TRIANGLE:         return 3;
  case TESS_QUAD:             return 4;
  case TESS_POLYVERTEX:
  case TESS_POLYLINE:
  case TESS_POLYGON:
  case TESS_TRIANGLE_STRIP:
                              return this->m_conn[offset + 1];
  default:
  case TESS_INVALID_CELL:     break;
    }
  return 0;
}

Tessellation::size_type Tessellation::vertexIdsOfCell(size_type offset, std::vector<int>& cellConn) const
{
  size_type cell_type;
  size_type num_verts = this->numberOfCellVertices(offset, &cell_type);
  ++ offset;
  if (cell_type & TESS_VARYING_VERT_CELL)
    ++ offset; // advance to first vertex.
  cellConn.insert(cellConn.end(), &this->m_conn[offset], &this->m_conn[offset + num_verts]);
  return num_verts;
}

Tessellation::size_type Tessellation::materialIdOfCell(size_type offset) const
{
  size_type cell_type;
  size_type num_verts = this->numberOfCellVertices(offset, &cell_type);
  if (!(cell_type & TESS_FACE_MATERIAL)) return -1;

  ++ offset;
  if (cell_type & TESS_VARYING_VERT_CELL)
    ++ offset; // advance to first vertex.
  offset += num_verts; // advance past vertices
  return this->m_conn[offset];
}

Tessellation::size_type Tessellation::insertNextCell(std::vector<int>& cellConn)
{
  size_type insert_pos = static_cast<size_type>(this->m_conn.size());
  return
    this->insertCell(insert_pos, cellConn) ?
    insert_pos :
    this->end();
}

bool Tessellation::insertCell(size_type offset, std::vector<int>& cellConn)
{
  size_type conn_length = cellConn.size();
  if (conn_length < 2) // Must have cell type plus at least one vertex ID
    return false;

  size_type num_verts;
  size_type cell_type = cellConn[0];
  size_type cell_shape = this->cellShapeFromType(cell_type);
  switch (cell_shape)
    {
  case TESS_VERTEX:           num_verts = 1; break;
  case TESS_TRIANGLE:         num_verts = 3; break;
  case TESS_QUAD:             num_verts = 4; break;
  case TESS_POLYVERTEX:
  case TESS_POLYLINE:
  case TESS_POLYGON:
  case TESS_TRIANGLE_STRIP:
                              num_verts = cellConn[1]; break;
  default:
  case TESS_INVALID_CELL:     return false;
    }

  // Determine whether cellConn is the proper length.
  // If not, then stop. Otherwise, insert more crud.
  int num_cell_props = this->numCellPropsFromType(cell_type);
  int num_vert_props = this->numVertexPropsFromType(cell_type);
  size_type expected_length =
    1 + // cell type
    ((cell_type & TESS_VARYING_VERT_CELL) ? 1 : 0) + // number of verts (when required)
    num_cell_props + // per-cell property offsets
    num_verts * (1 + num_vert_props); // cell connectivity + per-vertex property offsets

  if (conn_length != expected_length)
    return false;

  std::vector<int>::iterator cur_insert = this->m_conn.begin() + offset;
  this->m_conn.insert(cur_insert, cellConn.begin(), cellConn.end());
  return true;
}

Tessellation::size_type Tessellation::cellShapeFromType(size_type ctype)
{
  return ctype & TESS_CELLTYPE_MASK;
}

int Tessellation::numCellPropsFromType(size_type ctype)
{
  int num_cell_props = 0;
  if (ctype & TESS_FACE_MATERIAL)      ++num_cell_props;
  if (ctype & TESS_FACE_UV)            ++num_cell_props;
  if (ctype & TESS_FACE_NORMAL)        ++num_cell_props;
  if (ctype & TESS_FACE_COLOR)         ++num_cell_props;
  return num_cell_props;
}

int Tessellation::numVertexPropsFromType(size_type ctype)
{
  int num_vert_props = 0;
  if (ctype & TESS_FACE_VERTEX_UV)     ++num_vert_props;
  if (ctype & TESS_FACE_VERTEX_NORMAL) ++num_vert_props;
  if (ctype & TESS_FACE_VERTEX_COLOR)  ++num_vert_props;
  return num_vert_props;
}

  } // model namespace
} // smtk namespace

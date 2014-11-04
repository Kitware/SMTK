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

/// Add a 3-D point coordinate to the tessellation, but not a vertex record.
int Tessellation::addCoords(double* a)
{
  std::vector<double>::size_type ipt = this->m_coords.size();
  for (int i = 0; i < 3; ++i)
    {
    this->m_coords.push_back(a[i]);
    }
  return static_cast<int>(ipt / 3);
}

/// Add a 3-D point coordinate to the tessellation, but not a vertex record.
Tessellation& Tessellation::addCoords(double x, double y, double z)
{
  this->m_coords.push_back(x);
  this->m_coords.push_back(y);
  this->m_coords.push_back(z);
  return *this;
}

/// Add a 3-D point coordinate to the tessellation plus a vertex record.
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

/// Add two 3-D point coordinates to the tessellation plus a line-segment record.
Tessellation& Tessellation::addLine(double* a, double* b)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  this->m_conn.push_back(TESS_POLYLINE);
  this->m_conn.push_back(2);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  return *this;
}

/// Add three 3-D point coordinates to the tessellation plus a triangle record.
Tessellation& Tessellation::addTriangle(double* a, double* b, double* c)
{
  std::vector<int> conn;
  conn.reserve(4);
  conn.push_back(TESS_TRIANGLE);
  conn.push_back(this->addCoords(a));
  conn.push_back(this->addCoords(b));
  conn.push_back(this->addCoords(c));
  this->insertNextCell(conn);
  return *this;
}

/// Add four 3-D point coordinates to the tessellation plus a quadrilateral record.
Tessellation& Tessellation::addQuad(double* a, double* b, double* c, double* d)
{
  std::vector<int> conn;
  conn.reserve(5);
  conn.push_back(TESS_QUAD);
  conn.push_back(this->addCoords(a));
  conn.push_back(this->addCoords(b));
  conn.push_back(this->addCoords(c));
  conn.push_back(this->addCoords(d));
  this->insertNextCell(conn);
  return *this;
}

/// Add a vertex record using a pre-existing point coordinate (referenced by ID).
Tessellation& Tessellation::addPoint(int ai)
{
  this->m_conn.push_back(TESS_VERTEX);
  this->m_conn.push_back(ai);
  return *this;
}

/// Add a line-segment record using 2 pre-existing point coordinates (referenced by ID).
Tessellation& Tessellation::addLine(int ai, int bi)
{
  this->m_conn.push_back(TESS_POLYLINE);
  this->m_conn.push_back(2);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  return *this;
}

/// Add a triangle record using 3 pre-existing point coordinates (referenced by ID).
Tessellation& Tessellation::addTriangle(int ai, int bi, int ci)
{
  this->m_conn.push_back(TESS_TRIANGLE);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  this->m_conn.push_back(ci);
  return *this;
}

/// Add a quadrilateral record using 4 pre-existing point coordinates (referenced by ID).
Tessellation& Tessellation::addQuad(int ai, int bi, int ci, int di)
{
  this->m_conn.push_back(TESS_QUAD);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  this->m_conn.push_back(ci);
  this->m_conn.push_back(di);
  return *this;
}

/// Erase all point coordinates and tessellation primitive records.
Tessellation& Tessellation::reset()
{
  this->m_conn.clear();
  this->m_coords.clear();
  return *this;
}

/**\brief Return an offset-style iterator for traversing the
  *       tessellation-primitive-record connectivity array.
  *
  * The integer that is returned can be used as an offset into the
  * connectivity array that marks the beginning of a tessellation
  * primitive (vertex, line segment, triangle, quad, etc.).
  */
Tessellation::size_type Tessellation::begin() const
{
  return this->m_conn.empty() ? this->end() : 0;
}

/**\brief Return an offset-style end iterator for traversing the
  *       tessellation-primitive-record connectivity array.
  *
  * This is just a special invalid value that makes offsets into
  * the connectivity array appear to be usable as iterators.
  */
Tessellation::size_type Tessellation::end() const
{ return -1; }

/**\brief Advance an offset into the connectivity record to the next cell.
  */
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

/**\brief Return the type of tessellation primitive stored at the given \a offset.
  *
  * Note that the cell type includes both its shape plus flags
  * used by Three.js to indicate what per-cell and/or per-vertex
  * properties are stored in addition to coordinates (e.g.,
  * normals, color, material).
  */
Tessellation::size_type Tessellation::cellType(size_type offset) const
{
  return
    (offset < 0 || offset >= static_cast<int>(this->m_conn.size())) ?
    TESS_INVALID_CELL :
    this->m_conn[offset];
}

/**\brief Return the number of vertices in the tessellation primitive
  *       stored at the given \a offset.
  *
  * You may optionally pass the address of \a cellTypeOut to obtain
  * the cell's type at the same time (since it must be fetched in
  * order to determine the number of vertices.
  */
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

/**\brief Return the vertex IDs of the points defining the tessellation
  *       primitive at the given \a offset in \a cellConn.
  *
  * Note that this is **not** the same as the vector of integers you
  * must pass the insertCell or insertNextCell methods!
  * This call populates \a cellConn with *just* the vertex IDs, not
  * the cell type, the number of vertices, or any non-vertex IDs stored
  * in the connectivity (such as normal-vector IDs, color IDs, material
  * IDs, etc.).
  */
Tessellation::size_type Tessellation::vertexIdsOfCell(size_type offset, std::vector<int>& cellConn) const
{
  size_type cell_type;
  size_type num_verts = this->numberOfCellVertices(offset, &cell_type);
  ++ offset;
  if (cell_type & TESS_VARYING_VERT_CELL)
    ++ offset; // advance to first vertex.
  cellConn.insert(cellConn.end(), &this->m_conn[offset], &this->m_conn[offset] + num_verts);
  return num_verts;
}

/// Return the material ID of the primitive at the given \a offset.
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

/**\brief Insert by specifying exactly the values
  *       to be appended to the end of the connectivity array.
  *
  * This simply calls insertCell with the proper offset.
  */
Tessellation::size_type Tessellation::insertNextCell(std::vector<int>& cellConn)
{
  size_type insert_pos = static_cast<size_type>(this->m_conn.size());
  return
    this->insertCell(insert_pos, cellConn) ?
    insert_pos :
    this->end();
}

/**\brief Insert by specifying exactly the values
  *       to be appended to the end of the connectivity array.
  *
  * This simply calls insertCell with the proper offset.
  */
Tessellation::size_type Tessellation::insertNextCell(size_type connLen, const int* cellConn)
{
  size_type insert_pos = static_cast<size_type>(this->m_conn.size());
  return
    this->insertCell(insert_pos, connLen, cellConn) ?
    insert_pos :
    this->end();
}

/**\brief Insert a cell by specifying exactly the values
  *       to be inserted into the connectivity array.
  *
  * This variant accepts vectors of integers rather than a pointer.
  */
bool Tessellation::insertCell(size_type offset, std::vector<int>& cellConn)
{
  size_type conn_length = cellConn.size();
  return
    conn_length > 0 ?
      this->insertCell(offset, conn_length, &cellConn[0]) :
      false;
}

/**\brief Insert a cell by specifying exactly the values
  *       to be inserted into the connectivity array.
  *
  * Note that this is not simply a list of point coordinate IDs to serve
  * as corner vertices for the primitive;
  * rather, it starts with the cell type,
  * then includes the number of vertices in the cell (but only when
  * the cell type requires it -- see TESS_VARYING_VERT_CELL),
  * followed by
  * the vertex IDs,
  * the material ID (if the TESS_FACE_MATERIAL bit is set in the cell type),
  * the per-cell UV coordinate ID (if the TESS_FACE_UV bit is set),
  * the per-vertex UV coordinate IDs (if the TESS_FACE_VERTEX_UV bit is set),
  * the per-cell normal vector ID (if the TESS_FACE_NORMAL bit is set),
  * the per-vertex normal vector IDs (if the TESS_FACE_VERTEX_NORMAL bit is set),
  * the per-cell color ID (if the TESS_FACE_COLOR bit is set), and
  * the per-vertex color IDs (if the TESS_FACE_VERTEX_COLOR bit is set).
  */
bool Tessellation::insertCell(size_type offset, size_type conn_length, const int* cellConn)
{
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
  this->m_conn.insert(cur_insert, cellConn, cellConn + conn_length);
  return true;
}

/// Given a cell type, return just the bits that represent its shape.
Tessellation::size_type Tessellation::cellShapeFromType(size_type ctype)
{
  return ctype & TESS_CELLTYPE_MASK;
}

/// Given a cell type, return the number of per-cell IDs stored in the connectivity array.
int Tessellation::numCellPropsFromType(size_type ctype)
{
  int num_cell_props = 0;
  if (ctype & TESS_FACE_MATERIAL)      ++num_cell_props;
  if (ctype & TESS_FACE_UV)            ++num_cell_props;
  if (ctype & TESS_FACE_NORMAL)        ++num_cell_props;
  if (ctype & TESS_FACE_COLOR)         ++num_cell_props;
  return num_cell_props;
}

/**\brief Given a cell type, return the number of per-vertex property IDs
  *       (not including vertex IDs) stored in the connectivity array.
  */
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

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

#include <cfloat>
#include <iostream>

namespace smtk
{
namespace model
{

Tessellation::Tessellation() = default;

/// Add a 3-D point coordinate to the tessellation, but not a vertex record.
int Tessellation::addCoords(const double* a)
{
  std::vector<double>::size_type ipt = m_coords.size();
  for (int i = 0; i < 3; ++i)
  {
    m_coords.push_back(a[i]);
  }
  return static_cast<int>(ipt / 3);
}

/// Add a 3-D point coordinate to the tessellation, but not a vertex record.
Tessellation& Tessellation::addCoords(double x, double y, double z)
{
  m_coords.push_back(x);
  m_coords.push_back(y);
  m_coords.push_back(z);
  return *this;
}

/// Add a 3-D point coordinate to the tessellation plus a vertex record.
Tessellation& Tessellation::addPoint(const double* a)
{
  int ai = this->addCoords(a);
  return this->addPoint(ai);
}

/// Add two 3-D point coordinates to the tessellation plus a line-segment record.
Tessellation& Tessellation::addLine(const double* a, const double* b)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  return this->addLine(ai, bi);
}

/// Add three 3-D point coordinates to the tessellation plus a triangle record.
Tessellation& Tessellation::addTriangle(const double* a, const double* b, const double* c)
{
  return this->addTriangle(this->addCoords(a), this->addCoords(b), this->addCoords(c));
}

/// Add four 3-D point coordinates to the tessellation plus a quadrilateral record.
Tessellation&
Tessellation::addQuad(const double* a, const double* b, const double* c, const double* d)
{
  return this->addQuad(
    this->addCoords(a), this->addCoords(b), this->addCoords(c), this->addCoords(d));
}

/// Add a vertex record using a pre-existing point coordinate (referenced by ID).
Tessellation& Tessellation::addPoint(int ai)
{
  m_conn.push_back(TESS_VERTEX);
  m_conn.push_back(ai);
  return *this;
}

/// Add a line-segment record using 2 pre-existing point coordinates (referenced by ID).
Tessellation& Tessellation::addLine(int ai, int bi)
{
  m_conn.push_back(TESS_POLYLINE);
  m_conn.push_back(2);
  m_conn.push_back(ai);
  m_conn.push_back(bi);
  return *this;
}

/// Add a triangle record using 3 pre-existing point coordinates (referenced by ID).
Tessellation& Tessellation::addTriangle(int ai, int bi, int ci)
{
  m_conn.push_back(TESS_TRIANGLE);
  m_conn.push_back(ai);
  m_conn.push_back(bi);
  m_conn.push_back(ci);
  return *this;
}

/// Add a quadrilateral record using 4 pre-existing point coordinates (referenced by ID).
Tessellation& Tessellation::addQuad(int ai, int bi, int ci, int di)
{
  m_conn.push_back(TESS_QUAD);
  m_conn.push_back(ai);
  m_conn.push_back(bi);
  m_conn.push_back(ci);
  m_conn.push_back(di);
  return *this;
}

/// given the id of points, set points into coords
void Tessellation::setPoint(std::size_t id, const double* points)
{
  if (id <= m_coords.size())
  {
    this->coords()[3 * id] = points[0];
    this->coords()[3 * id + 1] = points[1];
    this->coords()[3 * id + 2] = points[2];
  }
}

/// Erase all point coordinates and tessellation primitive records.
Tessellation& Tessellation::reset()
{
  m_conn.clear();
  m_coords.clear();
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
  return m_conn.empty() ? this->end() : 0;
}

/**\brief Return an offset-style end iterator for traversing the
  *       tessellation-primitive-record connectivity array.
  *
  * This is just a special invalid value that makes offsets into
  * the connectivity array appear to be usable as iterators.
  */
Tessellation::size_type Tessellation::end() const
{
  return -1;
}

/**\brief Advance an offset into the connectivity record to the next cell.
  */
Tessellation::size_type Tessellation::nextCellOffset(size_type curOffset) const
{
  size_type cell_type;
  size_type num_verts = this->numberOfCellVertices(curOffset, &cell_type);
  if (num_verts == 0)
    return this->end();

  int num_cell_props = Tessellation::numCellPropsFromType(cell_type);
  int num_vert_props = Tessellation::numVertexPropsFromType(cell_type);

  size_type unchecked_next = curOffset + (cell_type & TESS_VARYING_VERT_CELL ? 2 : 1) +
    num_verts * (1 + num_vert_props) + num_cell_props;
  size_type next = (unchecked_next < 0 || unchecked_next >= static_cast<int>(m_conn.size()))
    ? this->end()
    : unchecked_next;
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
  return (offset < 0 || offset >= static_cast<int>(m_conn.size())) ? TESS_INVALID_CELL
                                                                   : m_conn[offset];
}

/**\brief Return the number of vertices in the tessellation primitive
  *       stored at the given \a offset.
  *
  * You may optionally pass the address of \a cellTypeOut to obtain
  * the cell's type at the same time (since it must be fetched in
  * order to determine the number of vertices.
  */
Tessellation::size_type Tessellation::numberOfCellVertices(size_type offset, size_type* cellTypeOut)
  const
{
  size_type cell_type = this->cellType(offset);
  if (cellTypeOut)
    *cellTypeOut = cell_type;

  size_type cell_shape = Tessellation::cellShapeFromType(cell_type);
  switch (cell_shape)
  {
    case TESS_VERTEX:
      return 1;
    case TESS_TRIANGLE:
      return 3;
    case TESS_QUAD:
      return 4;
    case TESS_POLYVERTEX:
    case TESS_POLYLINE:
    case TESS_POLYGON:
    case TESS_TRIANGLE_STRIP:
      return m_conn[offset + 1];
    default:
    case TESS_INVALID_CELL:
      break;
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
Tessellation::size_type Tessellation::vertexIdsOfCell(size_type offset, std::vector<int>& cellConn)
  const
{
  size_type cell_type;
  size_type num_verts = this->numberOfCellVertices(offset, &cell_type);
  ++offset;
  if (cell_type & TESS_VARYING_VERT_CELL)
    ++offset; // advance to first vertex.
  cellConn.insert(cellConn.end(), &m_conn[offset], &m_conn[offset] + num_verts);
  return num_verts;
}

/// Return the material ID of the primitive at the given \a offset.
Tessellation::size_type Tessellation::materialIdOfCell(size_type offset) const
{
  size_type cell_type;
  size_type num_verts = this->numberOfCellVertices(offset, &cell_type);
  if (!(cell_type & TESS_FACE_MATERIAL))
    return -1;

  ++offset;
  if (cell_type & TESS_VARYING_VERT_CELL)
    ++offset;          // advance to first vertex.
  offset += num_verts; // advance past vertices
  return m_conn[offset];
}

/**\brief Populate \a first and \a last with the vertex IDs at each end of a polyline at \a offset.
  *
  * Note that when the cell at \a offset is not a polyline (or when the polyline
  * is degenerate by virtue of having 0 or 1 vertices),
  * this method will return false and \a first and \a last will be undefined.
  */
bool Tessellation::vertexIdsOfPolylineEndpoints(size_type offset, int& first, int& last) const
{
  size_type ct;
  size_type nv = this->numberOfCellVertices(offset, &ct);
  if (ct != TESS_POLYLINE || nv < 2)
  {
    return false;
  }
  first = m_conn[2];
  last = m_conn[nv + 1];
  return true;
}

/**\brief Insert by specifying exactly the values
  *       to be appended to the end of the connectivity array.
  *
  * This simply calls insertCell with the proper offset.
  */
Tessellation::size_type Tessellation::insertNextCell(std::vector<int>& cellConn)
{
  size_type insert_pos = static_cast<size_type>(m_conn.size());
  return this->insertCell(insert_pos, cellConn) ? insert_pos : this->end();
}

/**\brief Insert by specifying exactly the values
  *       to be appended to the end of the connectivity array.
  *
  * This simply calls insertCell with the proper offset.
  */
Tessellation::size_type Tessellation::insertNextCell(size_type connLen, const int* cellConn)
{
  size_type insert_pos = static_cast<size_type>(m_conn.size());
  return this->insertCell(insert_pos, connLen, cellConn) ? insert_pos : this->end();
}

/**\brief Insert a cell by specifying exactly the values
  *       to be inserted into the connectivity array.
  *
  * This variant accepts vectors of integers rather than a pointer.
  */
bool Tessellation::insertCell(size_type offset, std::vector<int>& cellConn)
{
  size_type conn_length = static_cast<size_type>(cellConn.size());
  return conn_length > 0 ? this->insertCell(offset, conn_length, cellConn.data()) : false;
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
  {
    std::cerr << "ERROR: conn length " << conn_length << " too short. Skipping.\n";
    return false;
  }

  size_type num_verts;
  size_type cell_type = cellConn[0];
  size_type cell_shape = Tessellation::cellShapeFromType(cell_type);
  switch (cell_shape)
  {
    case TESS_VERTEX:
      num_verts = 1;
      break;
    case TESS_TRIANGLE:
      num_verts = 3;
      break;
    case TESS_QUAD:
      num_verts = 4;
      break;
    case TESS_POLYVERTEX:
    case TESS_POLYLINE:
    case TESS_POLYGON:
    case TESS_TRIANGLE_STRIP:
      num_verts = cellConn[1];
      break;
    default:
    case TESS_INVALID_CELL:
      std::cerr << "ERROR: Unknown cell type " << cell_type << "\n";
      return false;
  }

  // Determine whether cellConn is the proper length.
  // If not, then stop. Otherwise, insert more crud.
  int num_cell_props = Tessellation::numCellPropsFromType(cell_type);
  int num_vert_props = Tessellation::numVertexPropsFromType(cell_type);
  size_type expected_length = 1 +                    // cell type
    ((cell_type & TESS_VARYING_VERT_CELL) ? 1 : 0) + // number of verts (when required)
    num_cell_props +                                 // per-cell property offsets
    num_verts * (1 + num_vert_props); // cell connectivity + per-vertex property offsets

  if (conn_length != expected_length)
  {
    std::cerr << "ERROR: Expected conn length " << expected_length << " got " << conn_length
              << ". Skipping.\n";
    return false;
  }

  std::vector<int>::iterator cur_insert = m_conn.begin() + offset;
  m_conn.insert(cur_insert, cellConn, cellConn + conn_length);
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
  if (ctype & TESS_FACE_MATERIAL)
    ++num_cell_props;
  if (ctype & TESS_FACE_UV)
    ++num_cell_props;
  if (ctype & TESS_FACE_NORMAL)
    ++num_cell_props;
  if (ctype & TESS_FACE_COLOR)
    ++num_cell_props;
  return num_cell_props;
}

/**\brief Given a cell type, return the number of per-vertex property IDs
  *       (not including vertex IDs) stored in the connectivity array.
  */
int Tessellation::numVertexPropsFromType(size_type ctype)
{
  int num_vert_props = 0;
  if (ctype & TESS_FACE_VERTEX_UV)
    ++num_vert_props;
  if (ctype & TESS_FACE_VERTEX_NORMAL)
    ++num_vert_props;
  if (ctype & TESS_FACE_VERTEX_COLOR)
    ++num_vert_props;
  return num_vert_props;
}

/**\brief Fill \a bbox with standard "invalid" bounds.
  *
  */
void Tessellation::invalidBoundingBox(double bbox[6])
{
  for (int cc = 0; cc < 3; ++cc)
  {
    bbox[2 * cc] = DBL_MAX;
    bbox[2 * cc + 1] = -DBL_MAX;
  }
}

/**\brief Compute the bounding box of the tessellation.
  *
  * This computes the per-axis bounds of all the point coordinates,
  * whether they are referenced by connectivity entries or not.
  * The return value is true when the tessellation has at least 1
  * point and false otherwise.
  * If false is returned, \a bbox is unmodified so that multiple
  * calls to getBoundingBox will only increase and never reset bounds;
  * this way the bbox for a collection can be obtained by calling
  * getBoundingBox() on each tessellation -- whether it is empty or not.
  */
bool Tessellation::getBoundingBox(double bbox[6]) const
{
  if (m_coords.empty())
  {
    return false;
  }

  std::vector<double>::const_iterator cit;
  int cc; // component being considered (x, y, z)

  // If the current bounds are invalid, set both min and max to the first point:
  if (bbox[0] > bbox[1])
  {
    for (cc = 0, cit = m_coords.begin(); cc < 3 && cit != m_coords.end(); ++cit, ++cc)
    {
      bbox[2 * cc] = *cit;
      bbox[2 * cc + 1] = *cit;
    }
    // If we only had 2 coordinates, the 3rd is assumed to be 0:
    for (; cc < 3; ++cc)
    {
      bbox[2 * cc] = bbox[2 * cc + 1] = 0.0;
    }
  }
  // Now update the bounds using all the coordinates we have:
  for (cc = 0, cit = m_coords.begin(); cit != m_coords.end(); ++cit, ++cc)
  {
    if (*cit < bbox[2 * (cc % 3)])
    { // Update min
      bbox[2 * (cc % 3)] = *cit;
    }
    else if (*cit > bbox[2 * (cc % 3) + 1])
    { // Update max
      bbox[2 * (cc % 3) + 1] = *cit;
    }
  }
  return true;
}

} // namespace model
} // namespace smtk

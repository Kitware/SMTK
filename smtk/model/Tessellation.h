//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Tessellation_h
#define __smtk_model_Tessellation_h

#include "smtk/SystemConfig.h"
#include "smtk/common/UUID.h"

#ifdef SMTK_HASH_STORAGE
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (push)
#    pragma warning (disable : 4996)  // Overeager "unsafe" parameter check
#  endif
#  include "sparsehash/sparse_hash_map"
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (pop)
#  endif
#else // SMTK_HASH_STORAGE
#  include <map>
#endif // SMTK_HASH_STORAGE


#include <vector>

namespace smtk {
  namespace model {

/**\brief Cell type information bit-vector constants.
  *
  * This enum holds specific bit-vector combinations used
  * by a subset of entries in a Tessellation instance's connectivity array;
  * the first connectivity entry of each cell in the connectivity array
  * uses these values and indicate how to interpret the rest of the
  * connectivity entries for the cell.
  *
  * Many of these correspond to Three.js bit values.
  * See https://github.com/mrdoob/three.js/wiki/JSON-Model-format-3
  * for more information.
  */
enum TessellationCellType
{
  TESS_VERTEX             = 0x0200, //!< Cell is a single vertex.
  TESS_TRIANGLE           = 0x0000, //!< Cell is a triangle (3 vertices, part of Three.JS).
  TESS_QUAD               = 0x0001, //!< Cell is a quadrilateral (4 vertices, part of Three.JS).
  TESS_POLYVERTEX         = 0x0400, //!< Cell is a polyvertex, with the number of vertices in the following connectivity entry.
  TESS_POLYLINE           = 0x0800, //!< Cell is a polyline, with the number of vertices in the following connectivity entry.
  TESS_POLYGON            = 0x1000, //!< Cell is a polygon, with the number of vertices in the following connectivity entry.
  TESS_TRIANGLE_STRIP     = 0x2000, //!< Cell is a triangle strip, with the number of vertices in the following connectivity entry.
  TESS_INVALID_CELL       = 0x4000, //!< Something went wrong; an invalid cell.

  TESS_VARYING_VERT_CELL  = 0x3c00, //!< All cell types that have a varying number of vertices (and store it after cell type).
  TESS_CELLTYPE_MASK      = 0x7e01, //!< The union of all cell type bits.

  TESS_FACE_MATERIAL      = 0x0002, //!< Cell has a material ID immediately following the list of vertices (part of Three.js).
  TESS_FACE_UV            = 0x0004, //!< Cell has an offset into the UV coordinate storage following material ID (part of Three.js).
  TESS_FACE_VERTEX_UV     = 0x0008, //!< Cell has N offsets into the UV coordinate storage following face UV (part of Three.js).
  TESS_FACE_NORMAL        = 0x0010, //!< Cell has an offset into the normal-vector storage following vertex UVs (part of Three.js).
  TESS_FACE_VERTEX_NORMAL = 0x0020, //!< Cell has N offsets into the normal-vector storage following face normal (part of Three.js).
  TESS_FACE_COLOR         = 0x0040, //!< Cell has an offset into the color storage following vertex normals (part of Three.js).
  TESS_FACE_VERTEX_COLOR  = 0x0080, //!< Cell has N offsets into the color storage following face color (part of Three.js).

  TESS_PROPERTY_MASK      = 0x00fe  //!< All properties that can be stored with a cell.
};

/**\brief Store geometric information related to model entities.
  *
  * This is currently used to store coordinates and connectivity of
  * a triangulation of the model entity for rendering.
  * However, it may also evolve to store information about the
  * underlying geometric construct being approximated.
  *
  * Each instance stores point coordinates and connectivity
  * in two different arrays.
  * These arrays are organized for maximum compatibility with
  * [ThreeJS](http://threejs.org/) but extended to allow additional
  * primitive types (mainly vertices and polylines, but also others).
  *
  * Although ThreeJS bit values are used to indicate additional
  * values stored in the connectivity (such as normal vector, color,
  * and uv-coordinate IDs), there is no storage for additional
  * properties (i.e., no normals, colors, or uv-coordinates).
  * That may change in the future.
  */
class SMTKCORE_EXPORT Tessellation
{
public:
  typedef int size_type;

  Tessellation();

  /// Direct access to the underlying point-coordinate storage
  std::vector<double>& coords()
    { return this->m_coords; }
  std::vector<double> const& coords() const
    { return this->m_coords; }

  /// Direct access to the underlying connectivity storage
  std::vector<int>& conn()
    { return this->m_conn; }
  std::vector<int> const& conn() const
    { return this->m_conn; }

  int addCoords(double* a);
  Tessellation& addCoords(double x, double y, double z);

  Tessellation& addPoint(double* a);
  Tessellation& addLine(double* a, double* b);
  Tessellation& addTriangle(double* a, double* b, double* c);
  Tessellation& addQuad(double* a, double* b, double* c, double* d);

  Tessellation& addPoint(int ai);
  Tessellation& addLine(int ai, int bi);
  Tessellation& addTriangle(int ai, int bi, int ci);
  Tessellation& addQuad(int ai, int bi, int ci, int di);

  Tessellation& reset();

  size_type begin() const;
  size_type end() const;
  size_type nextCellOffset(size_type curOffset) const;
  size_type cellType(size_type offset) const;
  size_type numberOfCellVertices(size_type offset, size_type* cellTypeOut) const;
  size_type vertexIdsOfCell(size_type offset, std::vector<int>& cellConn) const;
  size_type materialIdOfCell(size_type offset) const;

  // TODO: Implement access to UVs, normals, colors, etc.
  //size_type vertexUVIdsOfCell(size_type offset, std::vector<int>& vertUVs) const;
  //...
  // *** OR ***
  // a single method for obtaining any integer-valued property
  // bool cellProperty(size_type prop, size_type offset, std::vector<int>& propOffsets) const;
  // e.g., cellProperty(TESS_FACE_VERTEX_NORMAL, offset, vertNormalIds);
  //       cellProperty(TESS_FACE_COLOR,         offset, vertNormalIds);

  size_type insertNextCell(std::vector<int>& cellConn);
  size_type insertNextCell(size_type connLen, const int* cellConn);

  bool insertCell(size_type offset, std::vector<int>& cellConn);
  bool insertCell(size_type offset, size_type connLen, const int* cellConn);

  static size_type cellShapeFromType(size_type);
  static int numCellPropsFromType(size_type cellType);
  static int numVertexPropsFromType(size_type cellType);

protected:
  std::vector<double> m_coords;
  std::vector<int> m_conn;
};

#ifdef SMTK_HASH_STORAGE
typedef google::sparse_hash_map<smtk::common::UUID,Tessellation> UUIDsToTessellations;
typedef google::sparse_hash_map<smtk::common::UUID,Tessellation>::iterator UUIDWithTessellation;
#else // SMTK_HASH_STORAGE
typedef std::map<smtk::common::UUID,Tessellation> UUIDsToTessellations;
typedef std::map<smtk::common::UUID,Tessellation>::iterator UUIDWithTessellation;
#endif // SMTK_HASH_STORAGE

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Tessellation_h

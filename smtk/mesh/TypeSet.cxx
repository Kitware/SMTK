//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/TypeSet.h"

namespace
{
  smtk::mesh::DimensionTypes make_dim_types(const smtk::mesh::CellTypes& ctypes)
  {
  using namespace smtk::mesh;
  DimensionTypes dtype;
  dtype[Dims0] = ctypes[Vertex]; //only have Dims0 if we have Vertex
  dtype[Dims1] = ctypes[Line]; //only have Dims1 if we have Line
  //only have Dims2 if we have Triangle, Quad or Polygon
  dtype[Dims2] = ctypes[Triangle] ||
                 ctypes[Quad]     ||
                 ctypes[Polygon];
  //The rest determine our Dims3 value
  dtype[Dims3] = ctypes[Tetrahedron]  ||
                 ctypes[Pyramid]      ||
                 ctypes[Wedge]        ||
                 ctypes[Hexahedron];
  return dtype;
  }
}

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
TypeSet::TypeSet():
  m_cellTypes(),
  m_dimTypes(),
  m_hasMesh( false ),
  m_hasCell( false ),
  m_hasPoint( false )
{

}

//----------------------------------------------------------------------------
TypeSet::TypeSet( smtk::mesh::CellTypes ctypes,
                  bool hasM, bool hasC, bool hasP ):
  m_cellTypes(ctypes),
  m_dimTypes( make_dim_types(ctypes) ),
  m_hasMesh( hasM ),
  m_hasCell( hasC ),
  m_hasPoint( hasP )
{

}


//----------------------------------------------------------------------------
bool TypeSet::hasMeshes() const
{
  return this->m_hasMesh;
}

//----------------------------------------------------------------------------
bool TypeSet::hasCells() const
{
    return this->m_hasCell;
}

//----------------------------------------------------------------------------
bool TypeSet::hasPoints() const
{
  return this->m_hasPoint;
}

//----------------------------------------------------------------------------
bool TypeSet::hasDimension( smtk::mesh::DimensionType dt ) const
{
  return this->m_dimTypes[ dt ];
}

//----------------------------------------------------------------------------
bool TypeSet::hasCell( smtk::mesh::CellType ct ) const
{
  return this->m_cellTypes[ ct ];
}

}
}

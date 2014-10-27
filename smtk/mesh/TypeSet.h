//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_TypeSet_h
#define __smtk_mesh_TypeSet_h

//TypeSet is a helper class whose goal is to make it easier to determine
//what kind of information you would extract from a collection given an
//association
#include "smtk/SMTKCoreExports.h"
#include "smtk/mesh/QueryTypes.h"

namespace smtk {
namespace mesh {

class SMTKCORE_EXPORT TypeSet
{
public:
  //construct an empty TypeSet
  TypeSet();

  //dimensions are inferred by what cell types are enabled
  TypeSet( smtk::mesh::CellTypes ctypes,
           bool hasM,
           bool hasC);

  bool operator==( const TypeSet& other ) const;
  bool operator!=( const TypeSet& other ) const;

  bool hasMeshes() const;
  bool hasCells() const;

  //Dimension is related fully to the cell types that are passed in.
  //What this means is that we don't consider having points ( coordinates )
  //a reason to mark a dim of 0 to true, that only happens if you have
  //Vertex cell types
  bool hasDimension( smtk::mesh::DimensionType dt ) const;
  bool hasCell( smtk::mesh::CellType ct ) const;

  const smtk::mesh::CellTypes& cellTypes() const { return this->m_cellTypes; }

private:
  smtk::mesh::CellTypes m_cellTypes;
  smtk::mesh::DimensionTypes m_dimTypes;
  bool m_hasMesh;
  bool m_hasCell;
};


}
}

#endif

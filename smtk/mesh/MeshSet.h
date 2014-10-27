//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_MeshSet_h
#define __smtk_mesh_MeshSet_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/Handle.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/TypeSet.h"

namespace smtk {
namespace mesh {

//Represents a collection of meshes that have been constructed by a Collection
//We represent the collection of meshes by holding onto the parent entity
//and a vector/range of mesh entities
class SMTKCORE_EXPORT MeshSet
{
  friend MeshSet set_intersect( const MeshSet& a, const MeshSet& b);
  friend MeshSet set_difference( const MeshSet& a, const MeshSet& b);
  friend MeshSet set_union( const MeshSet& a, const MeshSet& b );
public:
  //construct a MeshSet that represents all meshes that are children
  //of the handle
  MeshSet(const smtk::mesh::CollectionPtr& parent,
          smtk::mesh::Handle handle);

  //construct a MeshSet that represents an arbitrary unkown subset meshes that
  //are children of the handle.
  MeshSet(const smtk::mesh::CollectionPtr& parent,
          smtk::mesh::Handle handle,
          const smtk::mesh::HandleRange& range);

  //Copy Constructor required for rule of 3
  MeshSet(const MeshSet& other);

  //required to be in the cpp file as we hold a HandleRange
  ~MeshSet();

  //Copy assignment operator required for rule of 3
  MeshSet& operator= (const MeshSet& other);
  bool operator==( const MeshSet& other ) const;
  bool operator!=( const MeshSet& other ) const;

  //append another MeshSet to this MeshSet, if the parents and collection
  //pointers don't match the append will return false
  bool append( const MeshSet& other);

  bool is_empty() const;
  std::size_t size() const;

  smtk::mesh::CellSet cells(); //all cells of the meshset
  smtk::mesh::PointSet points(); //all points of the meshset

  //we should be able to extract the points or cells of the meshes.
  smtk::mesh::CellSet   cells( smtk::mesh::CellType cellType );
  smtk::mesh::CellSet   cells( smtk::mesh::CellTypes cellTypes );
  smtk::mesh::CellSet   cells( smtk::mesh::DimensionType dim );

private:
  smtk::mesh::CollectionPtr m_parent;
  smtk::mesh::Handle m_handle;
  smtk::mesh::HandleRange m_range; //range of moab entity sets
};

//Function that provide set operations on MeshSets


//intersect two mesh sets, placing the results in the return mesh set
//Note: If the meshsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT MeshSet set_intersect( const MeshSet& a, const MeshSet& b);

//subtract mesh b from a, placing the results in the return mesh set
//Note: If the meshsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT MeshSet set_difference( const MeshSet& a, const MeshSet& b);

//union two mesh sets, placing the results in the return mesh set
//Note: If the meshsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT MeshSet set_union( const MeshSet& a, const MeshSet& b );


}
}

#endif

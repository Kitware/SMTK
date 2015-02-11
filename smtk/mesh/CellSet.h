//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_CellSet_h
#define __smtk_mesh_CellSet_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/PointConnectivity.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/TypeSet.h"

namespace smtk {
namespace mesh {

//Represents a collection of cells that have been constructed by a Collection
//We represent the collection of cells by range of cell id entities. CellSets
//are a fairly lightweight representation, that are meant to be subdivided
//intersected, etc to generate the subset of cells the caller is interested in.
//To get the actual Cell / Geometric information you need to use the
//points() calls on a CellSet
//Note CellSets aren't stored in the actual database, they are just local
//views of the data. If you need to save a CellSet you will need to create
//a new MeshSet using Collection::createMesh.
class SMTKCORE_EXPORT CellSet
{
  friend CellSet set_intersect( const CellSet& a, const CellSet& b);
  friend CellSet set_difference( const CellSet& a, const CellSet& b);
  friend CellSet set_union( const CellSet& a, const CellSet& b );
  friend CellSet point_intersect( const CellSet& a, const CellSet& b, ContainmentType t);
  friend CellSet point_difference( const CellSet& a, const CellSet& b, ContainmentType t);
  friend class Collection; //required for creation of new meshes, deletion of cells
public:

  //construct a CellSet that represents an arbitrary unknown subset of cells that
  //are children of the handle.
  CellSet(const smtk::mesh::CollectionPtr& parent,
          const smtk::mesh::HandleRange& range);

  //Copy Constructor required for rule of 3
  CellSet(const CellSet& other);

  //required to be in the cpp file as we hold a HandleRange
  ~CellSet();

  //Copy assignment operator required for rule of 3
  CellSet& operator= (const CellSet& other);
  bool operator==( const CellSet& other ) const;
  bool operator!=( const CellSet& other ) const;

  //append another CellSet to this CellSet, if the collection
  //pointers don't match the append will return false
  bool append( const CellSet& other);

  bool is_empty() const;
  std::size_t size() const;

  smtk::mesh::Points points(); //all points of the cellset
  smtk::mesh::PointConnectivity pointConnectivity( ); //all connectivity info for all cells

  //get the points for a single cell
  smtk::mesh::Points points( std::size_t ) const;
  //get the connectivity for a single cell
  smtk::mesh::PointConnectivity pointConnectivity( std::size_t ) const;

private:
  smtk::mesh::CollectionPtr m_parent;
  smtk::mesh::HandleRange m_range; //range of moab cell ids
};

//Function that provide set operations on CellSets


//intersect two cell sets, placing the results in the return cell set.
//This does cell Id level intersection, if you want to intersect
//based on shared points you want to use point_intersect
//Note: If the cellsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet set_intersect( const CellSet& a, const CellSet& b);

//subtract cell b from a, placing the results in the return cell set.
//This does cell Id level difference, if you want to subtract
//based on shared points you want to use point_difference
//Note: If the cellsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet set_difference( const CellSet& a, const CellSet& b);

//union two cell sets, placing the results in the return cell set
//Note: If the cellsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet set_union( const CellSet& a, const CellSet& b );


//intersect two cell sets at the point id level, all cells from b which
//share points with cells in a are placed in the resulting CellSet.
//Currently the two options are Partial Contained and Fully Contained,
//with the former meaning at least 1 point per cell needs to be
//in set 1 to be contained in the result, while the latter requires all points
//per cell.
//Note: If the cellsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet point_intersect( const CellSet& a, const CellSet& b,
                                         ContainmentType t);

//subtract two cell sets at the point id level, all cells from b whose
//points are not used by cells from a are placed in the resulting CellSet.
//Currently the two options are Partial Contained and Fully Contained,
//with the former meaning at least 1 point per cell causes it to be removed,
//while the latter requires all points per cell.
//Note: If the cellsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet point_difference( const CellSet& a, const CellSet& b,
                                          ContainmentType t);

}
}

#endif

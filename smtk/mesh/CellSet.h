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
class SMTKCORE_EXPORT CellSet
{
  friend CellSet set_intersect( const CellSet& a, const CellSet& b);
  friend CellSet set_difference( const CellSet& a, const CellSet& b);
  friend CellSet set_union( const CellSet& a, const CellSet& b );
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
  smtk::mesh::Connectivity connectivity( ); //all connectivity info for all cells

  //get the points for a single cell
  smtk::mesh::Points points( std::size_t ) const;
  //get the connectivity for a single cell
  smtk::mesh::Connectivity connectivity( std::size_t ) const;

private:
  smtk::mesh::CollectionPtr m_parent;
  smtk::mesh::HandleRange m_range; //range of moab cell ids
};

//Function that provide set operations on CellSets


//intersect two mesh sets, placing the results in the return mesh set
//Note: If the meshsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet set_intersect( const CellSet& a, const CellSet& b);

//subtract mesh b from a, placing the results in the return mesh set
//Note: If the meshsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet set_difference( const CellSet& a, const CellSet& b);

//union two mesh sets, placing the results in the return mesh set
//Note: If the meshsets come from different collections the result will
//always be empty
SMTKCORE_EXPORT CellSet set_union( const CellSet& a, const CellSet& b );


}
}

#endif

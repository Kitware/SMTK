//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <iostream>
#include "smtk/mesh/PointConnectivity.h"

#include "smtk/mesh/moab/Helpers.h"

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
class PointConnectivity::InternalStorageImpl
{
public:
  InternalStorageImpl(const smtk::mesh::moab::InterfacePtr& iface,
                      const smtk::mesh::HandleRange& cells):
    ConnectivityStartPositions(),
    ConnectivityArraysLengths(),
    ConnectivityVertsPerCell(),
    NumberOfCells(0),
    NumberOfVerts(0)
  {

  std::size_t cellCount = 0;
  std::size_t vertCount = 0;

  smtk::mesh::HandleRange::const_iterator cells_current = cells.begin();
  smtk::mesh::HandleRange::const_iterator cells_end = cells.end();

  while(cells_current != cells_end)
    {
    ::moab::EntityHandle* connectivity;
    int numVertsPerCell=0, numCellsInSubRange=0;

    ::moab::ErrorCode result =
    iface->connect_iterate(cells_current,
                           cells.end(),
                           connectivity,
                           numVertsPerCell,
                           numCellsInSubRange);


    if(result == ::moab::MB_SUCCESS)
      {
      this->ConnectivityStartPositions.push_back(connectivity);
      this->ConnectivityArraysLengths.push_back(numCellsInSubRange);
      this->ConnectivityVertsPerCell.push_back(numVertsPerCell);

      //increment our iterator
      cells_current += static_cast<std::size_t>(numCellsInSubRange);

      //increment our num cells and verts counters
      cellCount += static_cast<std::size_t>(numCellsInSubRange);
      vertCount += static_cast<std::size_t>(numCellsInSubRange * numVertsPerCell);
      }
    else
      {
      //we shouldn't presume all ids coming in have connectivity. In theory
      //if we are passed spectral elements we will get back no connectivity.

      // std::cerr << "Passed range that contained non cell ids" << std::endl;
      // std::cerr << "Handle ids " << *cells_current.start_of_block()
      //                            << " to "
      //                            << *cells_current.end_of_block()
      //                            << "are not cell ids" << std::endl;
      // iface->list_entity(*cells_current.start_of_block());

      //increment our iterator to skip all these bad ids
      cells_current = cells_current.end_of_block();
      ++cells_current;
      }
    }

  this->NumberOfCells = cellCount;
  this->NumberOfVerts = vertCount;
  }

  InternalStorageImpl(const smtk::mesh::moab::InterfacePtr& iface,
                      const smtk::mesh::Handle& cell):
    ConnectivityStartPositions(),
    ConnectivityArraysLengths(),
    ConnectivityVertsPerCell(),
    NumberOfCells(0),
    NumberOfVerts(0)
  {
  const ::moab::EntityHandle* connectivity;
  int numVertsPerCell=0;
  const int numCellsInSubRange=1; //we are only passed a single cell

  iface->get_connectivity(cell,
                          connectivity,
                          numVertsPerCell);

  this->ConnectivityStartPositions.push_back(connectivity);
  this->ConnectivityArraysLengths.push_back(numCellsInSubRange);
  this->ConnectivityVertsPerCell.push_back(numVertsPerCell);

  this->NumberOfCells = numCellsInSubRange;
  this->NumberOfVerts = numVertsPerCell;
  }

  std::size_t cellSize() const { return NumberOfCells; }

  std::size_t vertSize() const { return NumberOfVerts; }


  void initTraversal( PointConnectivity::IterationState& state )
  {
    state.whichConnectivityVector = 0;
    state.ptrOffsetInVector = 0;
  }

  bool fetchNextCell( PointConnectivity::IterationState& state,
                     int& numPts,
                     const smtk::mesh::Handle* &points)
  {
    const std::size_t index = state.whichConnectivityVector;
    const std::size_t ptr = state.ptrOffsetInVector;


    numPts = this->ConnectivityVertsPerCell[ index ];
    points = &this->ConnectivityStartPositions[ index ][ ptr ];

    //now determine if we can safely increment
    const std::size_t currentArrayLength = this->ConnectivityArraysLengths[index] *
                                           this->ConnectivityVertsPerCell[index];

    if( ptr + 1 >= currentArrayLength)
      {
      //move to the next vector
      ++state.whichConnectivityVector;
      state.ptrOffsetInVector = 0;

      //return true if we haven't iterated passed the end of connectivity pointers
      return state.whichConnectivityVector < this->ConnectivityVertsPerCell.size();
      }
    else
      {
      state.ptrOffsetInVector += numPts;
      return true;
      }
  }


  bool equal( InternalStorageImpl* other ) const
  {
    if( this == other ) { return true;}

    //two quick checks that can confirm two items aren't equal
    if( this->NumberOfCells != other->NumberOfCells )
      { return false; }

    if( this->ConnectivityStartPositions.size() !=
        other->ConnectivityStartPositions.size() )
      { return false; }

    //now we know that both sets have the same number of moab pointers
    //and the total number of cells are the same. So we just have too
    //iterate the handles and the array lengths and confirm they all match.
    //in theory you could have equal start positions but different lengths
    //that both sum up to the same total num cells.

    typedef std::vector< const ::moab::EntityHandle* >::const_iterator hc_it;
    typedef std::vector< int >::const_iterator ic_it;

    const hc_it this_end = this->ConnectivityStartPositions.end();

    hc_it this_ptr = this->ConnectivityStartPositions.begin();
    hc_it other_ptr = other->ConnectivityStartPositions.begin();

    ic_it this_len = this->ConnectivityArraysLengths.begin();
    ic_it other_len = other->ConnectivityArraysLengths.begin();

    while( this_ptr != this_end &&
           *this_ptr == *other_ptr &&
           *this_len == *other_len)
      {
      ++this_ptr;
      ++other_ptr;
      ++this_len;
      ++other_len;
      }

    //if we are at the end that means the collections match
    return this_ptr == this_end;
  }

private:
  std::vector< const smtk::mesh::Handle* > ConnectivityStartPositions;
  std::vector<int> ConnectivityArraysLengths;
  std::vector<int> ConnectivityVertsPerCell;
  std::size_t NumberOfCells;
  std::size_t NumberOfVerts;
};


//----------------------------------------------------------------------------
PointConnectivity::PointConnectivity(const smtk::mesh::CollectionPtr& parent,
                           const smtk::mesh::HandleRange& range):
  m_parent(parent),
  m_connectivity( new InternalStorageImpl(smtk::mesh::moab::extractInterface(parent), range) ),
  m_iteratorLocation()
{

}

//----------------------------------------------------------------------------
PointConnectivity::PointConnectivity(const smtk::mesh::CollectionPtr& parent,
                           const smtk::mesh::Handle& cell):
  m_parent(parent),
  m_connectivity( new InternalStorageImpl(smtk::mesh::moab::extractInterface(parent), cell) ),
  m_iteratorLocation()
{

}


//----------------------------------------------------------------------------
PointConnectivity::PointConnectivity(const smtk::mesh::PointConnectivity& other):
  m_parent(other.m_parent),
  m_connectivity(other.m_connectivity)
{

}

//----------------------------------------------------------------------------
PointConnectivity::~PointConnectivity()
{

}

//----------------------------------------------------------------------------
PointConnectivity& PointConnectivity::operator=(const PointConnectivity& other)
{
  this->m_parent = other.m_parent;
  this->m_connectivity = other.m_connectivity;
  return *this;
}

//----------------------------------------------------------------------------
bool PointConnectivity::operator==(const PointConnectivity& other) const
{
  return this->m_parent == other.m_parent &&
         this->m_connectivity->equal(other.m_connectivity.get());
}

//----------------------------------------------------------------------------
bool PointConnectivity::operator!=(const PointConnectivity& other) const
{
  return !(*this == other);
}

//----------------------------------------------------------------------------
std::size_t PointConnectivity::size( ) const
{
  return this->m_connectivity->vertSize();
}

//----------------------------------------------------------------------------
std::size_t PointConnectivity::numberOfCells( ) const
{
  return this->m_connectivity->cellSize();
}

//----------------------------------------------------------------------------
bool PointConnectivity::is_empty( ) const
{
  return this->m_connectivity->cellSize() == 0;
}

//----------------------------------------------------------------------------
void PointConnectivity::initCellTraversal()
{
  //we store the iteration of the traversal inside ourselves not inside
  //the connectivity. The primary reason for this is that the connectivity
  //can be shared between multiple instance of connectivity, but each one
  //should be able to iterate over the shared data

  this->m_connectivity->initTraversal( this->m_iteratorLocation );
}

//----------------------------------------------------------------------------
bool PointConnectivity::fetchNextCell(int& numPts, const smtk::mesh::Handle* &points)
{
  return this->m_connectivity->fetchNextCell(this->m_iteratorLocation,
                                             numPts,
                                             points);
}

}
}

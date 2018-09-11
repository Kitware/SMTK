//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtkdiscrete_DiscreteMeshCellIdIterator_h
#define __smtkdiscrete_DiscreteMeshCellIdIterator_h

#include <iterator>  //needed for custom iterator
#include <vtkType.h> //needed for vtkIdType

namespace detail
{
//explicitly inline to remove duplicate symbols
static inline vtkIdType transform_id(vtkIdType cellId, vtkIdType numEdges)
{
  const int is_negative = (static_cast<int>(cellId < numEdges));
  //the xor gets us the negative edge ids, and than we subtract zero
  //else for face ids the xor does nothing an than we subtract the numedges
  return (cellId ^ -is_negative) - (numEdges * !is_negative);
}
}

//this is an iterator that holds onto two cell arrays, and unions the two
//cell arrays together. Returns the cell Id for each cell
class DiscreteMeshCellIdIterator : public std::iterator<std::forward_iterator_tag, const vtkIdType>
{
private:
  vtkIdType NumEdges;
  vtkIdType TotalNumCells;
  vtkIdType CurrentIndex; //stores the index
  vtkIdType CurrentValue; //stores the transformed index
public:
  DiscreteMeshCellIdIterator(vtkIdType numEdges, vtkIdType numFaces, vtkIdType current = 0)
    : NumEdges(numEdges)
    , TotalNumCells(numEdges + numFaces)
    , CurrentIndex(current)
    , CurrentValue(detail::transform_id(current, numEdges))
  {
  }

  reference operator*() const { return this->CurrentValue; }
  pointer operator->() const { return &this->CurrentValue; }

  DiscreteMeshCellIdIterator& operator++()
  { //prefix
    ++CurrentIndex;
    CurrentValue = detail::transform_id(CurrentIndex, NumEdges);
    return *this;
  }

  DiscreteMeshCellIdIterator operator++(int)
  {
    DiscreteMeshCellIdIterator postFix(*this);
    ++postFix;
    return postFix;
  }

  bool operator==(DiscreteMeshCellIdIterator b) const
  {
    return this->CurrentIndex == b.CurrentIndex;
  }
  bool operator!=(DiscreteMeshCellIdIterator b) const
  {
    return this->CurrentIndex != b.CurrentIndex;
  }
};

#endif // __DiscreteMeshCellIdIterator_h

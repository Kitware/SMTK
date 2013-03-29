/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#ifndef __DiscreteMeshCellIdIterator_h
#define __DiscreteMeshCellIdIterator_h

#include <iterator> //needed for custom iterator
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
class DiscreteMeshCellIdIterator
   : public std::iterator<std::forward_iterator_tag, const vtkIdType>
{
private:
  vtkIdType NumEdges;
  vtkIdType TotalNumCells;
  vtkIdType CurrentIndex; //stores the index
  vtkIdType CurrentValue; //stores the transformed index
public:
    DiscreteMeshCellIdIterator(vtkIdType numEdges, vtkIdType numFaces,
                               vtkIdType current=0):
      NumEdges(numEdges),
      TotalNumCells(numEdges + numFaces),
      CurrentIndex(current),
      CurrentValue(detail::transform_id(current,numEdges))
    {
    }

    reference operator*() const { return this->CurrentValue; }
    pointer operator->() const { return &this->CurrentValue; }

    DiscreteMeshCellIdIterator& operator++()
      {//prefix
      ++CurrentIndex;
      CurrentValue = detail::transform_id(CurrentIndex,NumEdges);
      return *this;
      }

    DiscreteMeshCellIdIterator operator++(int)
      {
      DiscreteMeshCellIdIterator postFix(*this);
      ++postFix;
      return postFix;
      }

    bool operator==(DiscreteMeshCellIdIterator b) const
      { return this->CurrentIndex == b.CurrentIndex; }
    bool operator!=(DiscreteMeshCellIdIterator b) const
      { return this->CurrentIndex != b.CurrentIndex; }
};

#endif // __DiscreteMeshCellIdIterator_h

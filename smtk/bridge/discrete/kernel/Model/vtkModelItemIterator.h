//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelItemIterator - Abstract interface for iterating over vtkModelItems
// .SECTION Description
// Abstract class for iterating over vtkModelItems.
// The intended use is:
// \code
//   vtkModelItemIterator* iterator = ModelItem->NewIterator(itemType);
//   for(iterator->Begin();!iterator->IsAtEnd();iterator->Next())
//     {
//       vtkModelItem* associatedItem = iterator->GetCurrentItem();
//     }
//   iterator->Delete();
// \endcode
// Note that the iterator must be deleted when the user is done
// using it to avoid a memory leak.

#ifndef __smtkdiscrete_vtkModelItemIterator_h
#define __smtkdiscrete_vtkModelItemIterator_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

class vtkModelItem;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelItemIterator : public vtkObject
{
public:
  vtkTypeMacro(vtkModelItemIterator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Go to the first item with given type.
  virtual void Begin() = 0;

  // Description:
  // Is the iterator at the end of the list.
  virtual int IsAtEnd() = 0;

  // Description:
  // Move to the next iterator.
  virtual void Next() = 0;

  // Description:
  // Returns the current item.
  virtual vtkModelItem* GetCurrentItem() = 0;

  // Description:
  // Returns the number of items being iterated over.
  virtual int Size() = 0;

protected:
  vtkModelItemIterator();
  virtual ~vtkModelItemIterator();

private:
  vtkModelItemIterator(const vtkModelItemIterator&);  // Not implemented.
  void operator=(const vtkModelItemIterator&);  // Not implemented.
};

#endif


//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelItemListIterator - Class for iterating over associated vtkModelItems
// .SECTION Description
// Abstract class for iterating over associated objects of a vtkModelItem.
// The intended use is:
// \code
//   vtkModelItemListIterator* iterator = ModelItem->NewIterator(itemType);
//   for(iterator->Begin();!iterator->IsAtEnd();iterator->Next())
//     {
//       vtkModelItem* associatedItem = iterator->GetCurrentItem();
//     }
//   iterator->Delete();
// \endcode
// Note that the iterator must be deleted when the user is done
// using it to avoid a memory leak.

#ifndef __smtkcmb_vtkModelItemListIterator_h
#define __smtkcmb_vtkModelItemListIterator_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelItemIterator.h"


//BTX
class vtkModelItem;
struct vtkModelItemListIteratorInternals;
//ETX

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelItemListIterator : public vtkModelItemIterator
{
public:
  static vtkModelItemListIterator *New();
  vtkTypeMacro(vtkModelItemListIterator,vtkModelItemIterator);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get functions for Root.  Root is set by the function that
  // creates the iterator.
  void SetRoot(vtkModelItem* item);
  vtkGetMacro(Root, vtkModelItem*);

  // Description:
  // Set/get functions for the itemType to iterate over.  This is set by the function
  // that creates the iterator.
  void SetItemType(int itemType);
  int GetItemType();

  // Description:
  // Go to the first item with given type.
  void Begin();

  // Description:
  // Is the iterator at the end of the list.
  int IsAtEnd();

  // Description:
  // Move to the next iterator.
  void Next();

  // Description:
  // Returns the current item.
  vtkModelItem* GetCurrentItem();

  // Description:
  // Returns the number of items being iterated over.
  virtual int Size();

protected:
  vtkModelItemListIterator();
  virtual ~vtkModelItemListIterator();

private:
  vtkModelItemListIterator(const vtkModelItemListIterator&);  // Not implemented.
  void operator=(const vtkModelItemListIterator&);  // Not implemented.

//BTX
  // Description:
  // Root is the object that will have its associated model entities
  // iterated over.
  vtkModelItem* Root;
  vtkModelItemListIteratorInternals* Internal;
//ETX
};

#endif


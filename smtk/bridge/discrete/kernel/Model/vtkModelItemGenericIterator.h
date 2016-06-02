//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelItemGenericIterator - Class for iterating over objects.
// .SECTION Description
// Class for iterating over objects that are explicitly put in the group.
// Note that the group is allowed to have an object listed multiple times.
// The objects will be iterated over in the order they were added.
// The intended use is:
// \code
//   vtkModelItemGenericIterator* iterator = ModelItem->NewIterator(itemType);
//   for(iterator->Begin();!iterator->IsAtEnd();iterator->Next())
//     {
//       vtkModelItem* associatedItem = iterator->GetCurrentItem();
//     }
//   iterator->Delete();
// \endcode
// Note that the iterator must be deleted when the user is done
// using it to avoid a memory leak.

#ifndef __smtkdiscrete_vtkModelItemGenericIterator_h
#define __smtkdiscrete_vtkModelItemGenericIterator_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelItemIterator.h"

class vtkModelItem;
struct vtkModelItemGenericIteratorInternals;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelItemGenericIterator : public vtkModelItemIterator
{
public:
  static vtkModelItemGenericIterator *New();
  vtkTypeMacro(vtkModelItemGenericIterator,vtkModelItemIterator);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Go to the first item with given type.
  virtual void Begin();

  // Description:
  // Is the iterator at the end of the list.
  virtual int IsAtEnd();

  // Description:
  // Move to the next iterator.
  virtual void Next();

  // Description:
  // Returns the current item.
  virtual vtkModelItem* GetCurrentItem();

  // Description:
  // Add a model item to the group of objects to be iterated over.
  void AddModelItem(vtkModelItem* modelItem);

  // Description:
  // Add a model item to the group of objects to be iterated over if it
  // is not already in the group.
  void AddUniqueModelItem(vtkModelItem* modelItem);

  // Description:
  // Remove all instances of an object from the group.
  void RemoveModelItem(vtkModelItem* modelItem);

  // Description:
  // Clear objects in group and reset the current iterator.
  void RemoveAllModelItems();

  // Description:
  // Returns the number of items being iterated over.
  virtual int Size();

protected:
  vtkModelItemGenericIterator();
  virtual ~vtkModelItemGenericIterator();

private:
  vtkModelItemGenericIterator(const vtkModelItemGenericIterator&);  // Not implemented.
  void operator=(const vtkModelItemGenericIterator&);  // Not implemented.

  // Description:
  // Container for the objects to be iterated over.
  vtkModelItemGenericIteratorInternals* Internal;

};

#endif


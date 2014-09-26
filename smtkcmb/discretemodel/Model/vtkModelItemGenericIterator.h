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

#ifndef __vtkModelItemGenericIterator_h
#define __vtkModelItemGenericIterator_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelItemIterator.h"
#include "cmbSystemConfig.h"

//BTX
class vtkModelItem;
struct vtkModelItemGenericIteratorInternals;
//ETX

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

//BTX
  // Description:
  // Container for the objects to be iterated over.
  vtkModelItemGenericIteratorInternals* Internal;
//ETX
};

#endif


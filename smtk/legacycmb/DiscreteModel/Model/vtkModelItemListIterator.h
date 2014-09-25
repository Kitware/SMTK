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

#ifndef __vtkModelItemListIterator_h
#define __vtkModelItemListIterator_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkModelItemIterator.h"
#include "cmbSystemConfig.h"

//BTX
class vtkModelItem;
struct vtkModelItemListIteratorInternals;
//ETX

class VTKDISCRETEMODEL_EXPORT vtkModelItemListIterator : public vtkModelItemIterator
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


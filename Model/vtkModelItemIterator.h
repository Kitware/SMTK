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

#ifndef __vtkModelItemIterator_h
#define __vtkModelItemIterator_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

//BTX
class vtkModelItem;
//ETX

class VTKDISCRETEMODEL_EXPORT vtkModelItemIterator : public vtkObject
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


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

#include "vtkModelItemListIterator.h"

#include "vtkModelItem.h"
#include "vtkModelItemInternals.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkModelItemListIterator, "");
vtkStandardNewMacro(vtkModelItemListIterator);

void vtkModelItemListIterator::SetRoot(vtkModelItem* item)
{
  this->Root = item;
}

struct vtkModelItemListIteratorInternals
{
  std::list<vtkSmartPointer<vtkModelItem> >::iterator
    ConceptualModelItemListIterator;
  int ItemType;
};

vtkModelItemListIterator::vtkModelItemListIterator()
{
  this->Internal = new vtkModelItemListIteratorInternals;
  this->Internal->ItemType = -1;
}

vtkModelItemListIterator::~vtkModelItemListIterator()
{
  delete this->Internal;
}

void vtkModelItemListIterator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkModelItemListIterator::SetItemType(int itemType)
{
  this->Internal->ItemType = itemType;
}

int vtkModelItemListIterator::GetItemType()
{
  return this->Internal->ItemType;
}

//---------------------------------------------------------------------------
void vtkModelItemListIterator::Begin()
{
  if (!this->Root)
    {
    vtkErrorMacro("Root is not set. Can not perform operation: Begin()");
    return;
    }
  if (this->Internal->ItemType == -1)
    {
    vtkErrorMacro("Type is not set. Can not perform operation: Begin()");
    return;
    }

  this->Internal->ConceptualModelItemListIterator = 
    this->Root->Internal->Associations[this->Internal->ItemType].begin();
}

//---------------------------------------------------------------------------
int vtkModelItemListIterator::IsAtEnd()
{
  if (!this->Root)
    {
    vtkErrorMacro("Root is not set. Can not perform operation: IsAtEnd()");
    return 1;
    }
  if ( this->Root->Internal->Associations.find(this->Internal->ItemType) ==
     this->Root->Internal->Associations.end() ||
       this->Internal->ConceptualModelItemListIterator == 
         this->Root->Internal->Associations[this->Internal->ItemType].end() )
    {
    return 1;
    }
  return 0;
}

//---------------------------------------------------------------------------
void vtkModelItemListIterator::Next()
{
  if (!this->Root)
    {
    vtkErrorMacro("Root is not set. Can not perform operation: Next()");
    return;
    }

  if (!this->IsAtEnd())
    {
    this->Internal->ConceptualModelItemListIterator++;
    return;
    }
}

//---------------------------------------------------------------------------
vtkModelItem* vtkModelItemListIterator::GetCurrentItem()
{
  if (!this->Root)
    {
    vtkErrorMacro("Root is not set. Can not perform operation: GetRoot()");
    return 0;
    }

  if (!this->IsAtEnd())
    {
    return this->Internal->ConceptualModelItemListIterator->GetPointer();
    }

  return 0;
}
//---------------------------------------------------------------------------
int vtkModelItemListIterator::Size()
{
  if (!this->Root)
    {
    vtkErrorMacro("Root is not set. Can not perform operation: Size()");
    return 0;
    }
  return static_cast<int>
    (this->Root->Internal->Associations[this->Internal->ItemType].size());
}



//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkModelItemListIterator.h"

#include "vtkModelItem.h"
#include "vtkModelItemInternals.h"
#include "vtkObjectFactory.h"

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



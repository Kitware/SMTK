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

#include "vtkModelItem.h"

#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkModelItemListIterator.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"
#include "vtkSmartPointer.h"

#include "vtkModelItemInternals.h"

vtkModelItem::vtkModelItem()
{
  this->Internal = new vtkModelItemInternals;
  this->Properties = vtkInformation::New();
}

vtkModelItem::~vtkModelItem()
{
  delete this->Internal;
  if (this->Properties)
    {
    this->Properties->Delete();
    }
}

void vtkModelItem::AddAssociation(vtkModelItem* item)
{
  this->AddAssociationToType(item, this->GetType());
}

void vtkModelItem::AddAssociationToType(vtkModelItem* item, int myType)
{
  this->Internal->Associations[item->GetType()].push_back(item);
  item->AddReverseAssociationToType(this,myType);
  this->Modified();
}

void vtkModelItem::AddAssociationInPosition(int index,
                                            vtkModelItem* item)
{
  const int itemType = item->GetType();
  const int numItems = this->GetNumberOfAssociations(itemType);
  if(numItems < index)
    {
    if((numItems+1) < index)
      {
      vtkWarningMacro("Possible bad Index value.");
      for(int i=numItems+1;i<index;i++)
        {
        // fill the proper places in just in case the user knows what
        // he/she is doing
        // use add reverse association since it won't attempt to add an
        // association to a null pointer
        this->AddReverseAssociationToType(NULL,itemType);
        }
      }
    this->AddAssociation(item);
    }
  else
    {
    std::list<vtkSmartPointer<vtkModelItem> >::iterator it=
      this->Internal->Associations[itemType].begin();
    int count = 0;
    while(count < index)
      {
      count++;
      it++;
      }
    this->Internal->Associations[itemType].insert(it, item);
    item->AddReverseAssociationToType(this,this->GetType());
    }
}

void vtkModelItem::AddReverseAssociationToType(vtkModelItem* item,
                                               int itemType)
{
  this->Internal->Associations[itemType].push_back(item);
  this->Modified();
}

void vtkModelItem::RemoveAllAssociations(int itemType)
{
  if(this->GetNumberOfAssociations(itemType) == 0)
    {
    return;
    }
  vtkModelItemIterator * iter = this->NewIterator(itemType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    iter->GetCurrentItem()->RemoveReverseAssociationToType(this,
                                                           this->GetType());
    }
  iter->Delete();
  this->Internal->Associations.erase(itemType);
  this->Modified();
}

void vtkModelItem::RemoveAssociation(vtkModelItem* item)
{
  const int itemType = item->GetType();
  if(this->Internal->Associations.find(itemType) !=
     this->Internal->Associations.end())
    {
    item->RemoveReverseAssociationToType(this,this->GetType());
    this->Internal->Associations[itemType].remove(item);
    this->Modified();
    }
}

void vtkModelItem::RemoveReverseAssociationToType(vtkModelItem* item,
                                                  int itemType)
{
  if(this->Internal->Associations.find(itemType) !=
     this->Internal->Associations.end())
    {
    this->Internal->Associations[itemType].remove(item);
    this->Modified();
    }
}

int vtkModelItem::GetNumberOfAssociations(int itemType)
{
  if(this->Internal->Associations.find(itemType) !=
     this->Internal->Associations.end())
    {
    return static_cast<int>(this->Internal->Associations[itemType].size());
    }
  return 0;
}

vtkModelItemIterator* vtkModelItem::NewIterator(int itemType)
{
  vtkModelItemListIterator* iter = vtkModelItemListIterator::New();
  iter->SetRoot(this);
  iter->SetItemType(itemType);
  return iter;
}

//---------------------------------------------------------------------------
void vtkModelItem::GetItemTypesList(vtkIdList * itemTypes)
{
  itemTypes->Reset();

  size_t size = this->Internal->Associations.size();
  itemTypes->Allocate(size);
  itemTypes->SetNumberOfIds(size);

  int counter = 0;
  for(vtkModelItemInternals::AssociationsMap::iterator it=
        this->Internal->Associations.begin();
      it!= this->Internal->Associations.end();it++)
    {
    itemTypes->InsertId(counter, it->first);
    counter++;
    }
}

void vtkModelItem::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
  ser->Serialize("Properties", this->Properties);

  if(ser->IsWriting())
    {
    std::map<int, std::vector<vtkSmartPointer<vtkObject> > > associations =
      vtkSerializer::ToBase<std::list<vtkSmartPointer<vtkModelItem> > > (
        this->Internal->Associations);
    ser->Serialize("Associations", associations);
    // may want to add in MTime in the near future so that we can do
    // incremental updates
    //unsigned long mtime = this->GetMTime();
    //ser->Serialize("MTime", mtime);
    }
}

void vtkModelItem::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ItemTypesAssociation:\n";
  vtkIndent indent2 = indent.GetNextIndent();
  vtkIdList* itemTypes = vtkIdList::New();
  this->GetItemTypesList(itemTypes);
  for(vtkIdType id=0;id<itemTypes->GetNumberOfIds();id++)
    {
    os << indent2 << "Type: " << itemTypes->GetId(id) << ", Quantity: "
       << this->GetNumberOfAssociations(itemTypes->GetId(id)) << "\n";
    }
  itemTypes->Delete();
}


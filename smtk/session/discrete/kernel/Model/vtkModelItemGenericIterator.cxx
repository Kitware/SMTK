//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelItemGenericIterator.h"

#include "vtkModelItem.h"
#include "vtkModelItemInternals.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkModelItemGenericIterator);

struct vtkModelItemGenericIteratorInternals
{
  typedef std::list<vtkSmartPointer<vtkModelItem> > Container;
  typedef Container::iterator ContainerIterator;
  Container Objects;
  ContainerIterator Iterator;
};

vtkModelItemGenericIterator::vtkModelItemGenericIterator()
{
  this->Internal = new vtkModelItemGenericIteratorInternals;
  this->Internal->Iterator = this->Internal->Objects.begin();
}

vtkModelItemGenericIterator::~vtkModelItemGenericIterator()
{
  delete this->Internal;
}

void vtkModelItemGenericIterator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void vtkModelItemGenericIterator::Begin()
{
  this->Internal->Iterator = this->Internal->Objects.begin();
}

int vtkModelItemGenericIterator::IsAtEnd()
{
  if (this->Internal->Iterator == this->Internal->Objects.end())
  {
    return 1;
  }
  return 0;
}

void vtkModelItemGenericIterator::Next()
{
  if (!this->IsAtEnd())
  {
    this->Internal->Iterator++;
    return;
  }
}

vtkModelItem* vtkModelItemGenericIterator::GetCurrentItem()
{
  if (!this->IsAtEnd())
  {
    return this->Internal->Iterator->GetPointer();
  }

  return 0;
}

void vtkModelItemGenericIterator::AddModelItem(vtkModelItem* modelItem)
{
  this->Internal->Objects.push_back(modelItem);
}

void vtkModelItemGenericIterator::AddUniqueModelItem(vtkModelItem* modelItem)
{
  for (vtkModelItemGenericIteratorInternals::ContainerIterator it = this->Internal->Objects.begin();
       it != this->Internal->Objects.end(); it++)
  {
    if (*it == modelItem)
    {
      return;
    }
  }
  this->Internal->Objects.push_back(modelItem);
}

void vtkModelItemGenericIterator::RemoveModelItem(vtkModelItem* modelItem)
{
  this->Internal->Objects.remove(modelItem);
}

void vtkModelItemGenericIterator::RemoveAllModelItems()
{
  this->Internal->Objects.clear();
  this->Internal->Iterator = this->Internal->Objects.begin();
}

int vtkModelItemGenericIterator::Size()
{
  return static_cast<int>(this->Internal->Objects.size());
}

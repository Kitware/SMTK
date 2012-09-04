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
  this->Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkModelItemGenericIterator::Begin()
{
  this->Internal->Iterator = this->Internal->Objects.begin();
}

//---------------------------------------------------------------------------
int vtkModelItemGenericIterator::IsAtEnd()
{
  if ( this->Internal->Iterator == this->Internal->Objects.end() )
    {
    return 1;
    }
  return 0;
}

//---------------------------------------------------------------------------
void vtkModelItemGenericIterator::Next()
{
  if (!this->IsAtEnd())
    {
    this->Internal->Iterator++;
    return;
    }
}

//---------------------------------------------------------------------------
vtkModelItem* vtkModelItemGenericIterator::GetCurrentItem()
{
  if (!this->IsAtEnd())
    {
    return this->Internal->Iterator->GetPointer();
    }

  return 0;
}

//---------------------------------------------------------------------------
void vtkModelItemGenericIterator::AddModelItem(vtkModelItem* ModelItem)
{
  this->Internal->Objects.push_back(ModelItem);
}

//---------------------------------------------------------------------------
void vtkModelItemGenericIterator::AddUniqueModelItem(vtkModelItem* ModelItem)
{
  for(vtkModelItemGenericIteratorInternals::ContainerIterator it=
        this->Internal->Objects.begin();
      it!=this->Internal->Objects.end();it++)
    {
    if(*it == ModelItem)
      {
      return;
      }
    }
  this->Internal->Objects.push_back(ModelItem);
}

//---------------------------------------------------------------------------
void vtkModelItemGenericIterator::RemoveModelItem(vtkModelItem* ModelItem)
{
  this->Internal->Objects.remove(ModelItem);
}

//---------------------------------------------------------------------------
void vtkModelItemGenericIterator::RemoveAllModelItems()
{
  this->Internal->Objects.clear();
  this->Internal->Iterator = this->Internal->Objects.begin();
}
//---------------------------------------------------------------------------
int vtkModelItemGenericIterator::Size()
{
  return this->Internal->Objects.size();
}
//---------------------------------------------------------------------------

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelItemIterator.h"

#include "vtkModelItem.h"
#include "vtkObjectFactory.h"

vtkModelItemIterator::vtkModelItemIterator()
{
}

vtkModelItemIterator::~vtkModelItemIterator()
{
}

void vtkModelItemIterator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

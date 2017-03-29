//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <list>
#include <map>

#include <vtkSmartPointer.h>

struct vtkModelItemInternals
{
  typedef std::map<int,
    std::list<vtkSmartPointer<vtkModelItem> > > AssociationsMap;
  AssociationsMap Associations;
};

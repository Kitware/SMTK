//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/ResourceManagerOperation.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace operation
{

smtk::resource::ManagerPtr ResourceManagerOperation::resourceManager()
{

  if (auto mgr = manager())
  {
    if (auto mgrs = mgr->managers())
    {
      if (mgrs->contains<smtk::resource::Manager::Ptr>())
      {
        return mgrs->get<smtk::resource::Manager::Ptr>();
      }
    }
  }

  return smtk::resource::ManagerPtr();
}
}
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/query/Manager.h"

namespace smtk
{
namespace resource
{
namespace query
{

Manager::Manager(const smtk::resource::ManagerPtr& manager)
  : m_manager(manager)
{
}

Manager::~Manager() = default;
}
}
}

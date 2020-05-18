//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/extension/vxl/operators/Registrar.h"

#include "smtk/extension/vxl/operators/smtkTerrainExtractionView.h"

namespace smtk
{
namespace extension
{
namespace vxl
{
namespace operators
{

namespace
{
typedef std::tuple<smtkTerrainExtractionView> ViewList;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerTypes<ViewList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterTypes<ViewList>();
}
}
}
}
}

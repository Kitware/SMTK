//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_delaunay_RegisterOperations_h
#define __smtk_extension_delaunay_RegisterOperations_h

#include "smtk/extension/delaunay/Exports.h"

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace extension
{
namespace delaunay
{
SMTKDELAUNAYEXT_EXPORT void registerOperations(smtk::operation::Manager::Ptr&);
}
}
}

#endif

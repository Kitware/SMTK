//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_delaunay_Registrar_h
#define __smtk_extension_delaunay_Registrar_h

#include "smtk/extension/delaunay/Exports.h"

#include "smtk/attribute/Registrar.h"
#include "smtk/mesh/resource/Registrar.h"
#include "smtk/model/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"

namespace smtk
{
namespace extension
{
namespace delaunay
{

class SMTKDELAUNAYEXT_EXPORT Registrar
{
public:
  using Dependencies =
    std::tuple<operation::Registrar, model::Registrar, attribute::Registrar, mesh::Registrar>;

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);
};
}
}
}

#endif

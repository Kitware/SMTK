//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonResourceAndRole.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonAdaptor.h"

#include "smtk/task/adaptor/ResourceAndRole.h"

namespace smtk
{
namespace task
{

using adaptor::ResourceAndRole;

namespace json
{

Adaptor::Configuration jsonResourceAndRole::operator()(const Adaptor* adaptor, Helper& helper) const
{
  Adaptor::Configuration config;
  auto* ncadaptor = const_cast<Adaptor*>(adaptor);
  auto* resourceAndRole = dynamic_cast<ResourceAndRole*>(ncadaptor);
  if (resourceAndRole)
  {
    jsonAdaptor superclass;
    config = superclass(resourceAndRole, helper);
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonConfigureOperation.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonAdaptor.h"

#include "smtk/task/adaptor/ConfigureOperation.h"

namespace smtk
{
namespace task
{

// using adaptor::ConfigureOperation;

namespace json
{

Adaptor::Configuration jsonConfigureOperation::operator()(const Adaptor* adaptor, Helper& helper)
  const
{
  Adaptor::Configuration config;
  auto* ncadaptor = const_cast<Adaptor*>(adaptor);
  auto* ConfigureOperation = dynamic_cast<adaptor::ConfigureOperation*>(ncadaptor);
  if (ConfigureOperation)
  {
    jsonAdaptor superclass;
    config = superclass(ConfigureOperation, helper);
    config["configure"] = ConfigureOperation->config();
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk

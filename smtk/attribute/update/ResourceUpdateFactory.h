//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_update_ResourceUpdateFactory_h
#define smtk_attribute_update_ResourceUpdateFactory_h

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/common/update/Factory.h"
#include "smtk/io/Logger.h"

namespace smtk
{
namespace attribute
{

class Resource;

namespace update
{

/// The signature of functions used to migrate an item across schema.
using ResourceUpdater = std::function<bool(const Resource&, Resource&, smtk::io::Logger&)>;

/**\brief Update an item in an attribute resource.
  *
  */
class SMTKCORE_EXPORT ResourceUpdateFactory : public smtk::common::update::Factory<ResourceUpdater>
{
public:
  smtkTypeMacroBase(smtk::attribute::update::ResourceUpdater);
};

} // namespace update
} // namespace attribute
} // namespace smtk

#endif // smtk_attribute_update_ResourceUpdateFactory_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_ImporterGroup_h
#define smtk_operation_ImporterGroup_h

#include "smtk/CoreExports.h"

#include "smtk/resource/Name.h"

#include "smtk/operation/Operation.h"
#include "smtk/operation/groups/ResourceIOGroup.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

class SMTKCORE_EXPORT ImporterGroup : public ResourceIOGroup
{
public:
  using ResourceIOGroup::registerOperation;

  static constexpr const char* const type_name = "importer";

  ImporterGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : ResourceIOGroup(type_name, manager)
  {
  }
};
}
}

#endif // smtk_operation_ImporterGroup_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/job/Definition.h"
#include "smtk/job/LogParser.h"
#include "smtk/job/Stage.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace
{
template<typename T>
void checkSharedFromThis()
{
  auto object = T::create();
  std::shared_ptr<T> shared = object->shared_from_this();
  test(shared == object, "shared_from_this must return the original object.");
  test(
    !shared.owner_before(object) && !object.owner_before(shared),
    "shared_from_this must preserve shared ownership.");

  const T& constObject = *object;
  std::shared_ptr<const T> constShared = constObject.shared_from_this();
  test(constShared == object, "Const shared_from_this must return the original object.");
  test(
    !constShared.owner_before(object) && !object.owner_before(constShared),
    "Const shared_from_this must preserve shared ownership.");
}
} // namespace

int jobSharedFromThis(int, char*[])
{
  checkSharedFromThis<smtk::job::Stage>();
  checkSharedFromThis<smtk::job::Definition>();
  checkSharedFromThis<smtk::job::LogParser>();
  return 0;
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_ExporterGroup_h
#define smtk_operation_ExporterGroup_h

#include "smtk/CoreExports.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Operation.h"
#include "smtk/operation/groups/ResourceIOGroup.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

class SMTKCORE_EXPORT ExporterGroup : public ResourceIOGroup
{
public:
  using ResourceIOGroup::registerOperation;

  static constexpr const char* const type_name = "exporter";

  ExporterGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : ResourceIOGroup(type_name, manager)
  {
  }

  // Given a file name, return the set of operators that accept the input file.
  std::set<Operation::Index> operationsForFileName(const std::string&) const;

  // Given a resource type and a file name, return the set of operators that
  // accept the input file and return a resource of the given type.
  template <typename ResourceType>
  std::set<Operation::Index> operationsForResourceAndFileName(const std::string&) const;

  // Given a resource name and a file name, return the set of operators that
  // accept the input file and return a resource of the given type.
  std::set<Operation::Index> operationsForResourceAndFileName(
    const std::string&, const std::string&) const;

private:
  // Given a set of operation indices, remove the ones that do not accept the
  // given file name.
  void filterOperationsThatRejectFileName(std::set<Operation::Index>&, const std::string&) const;
};

template <typename ResourceType>
std::set<Operation::Index> ExporterGroup::operationsForResourceAndFileName(
  const std::string& fileName) const
{
  return operationsForResourceAndFileName(smtk::common::typeName<ResourceType>(), fileName);
}
}
}

#endif // smtk_operation_ExporterGroup_h

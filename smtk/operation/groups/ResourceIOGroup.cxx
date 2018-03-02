//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/ResourceIOGroup.h"

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace operation
{

const std::string ResourceIOGroup::m_defaultFileItemName = "filename";

smtk::attribute::FileItem::Ptr ResourceIOGroup::fileItemForOperation(
  const std::string& uniqueName) const
{
  Operation::Specification spec = specification(uniqueName);
  if (spec == nullptr)
  {
    return smtk::attribute::FileItem::Ptr();
  }

  Operation::Parameters parameters = extractParameters(spec, uniqueName);
  return parameters->findFile(*(m_fileItemName.values(uniqueName).begin()));
}

smtk::attribute::FileItem::Ptr ResourceIOGroup::fileItemForOperation(
  const Operation::Index& index) const
{
  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return smtk::attribute::FileItem::Ptr();
  }

  auto metadata = manager->metadata().get<IndexTag>().find(index);
  if (metadata == manager->metadata().get<IndexTag>().end())
  {
    return smtk::attribute::FileItem::Ptr();
  }

  Operation::Specification spec = specification(metadata->uniqueName());
  if (spec == nullptr)
  {
    return smtk::attribute::FileItem::Ptr();
  }

  Operation::Parameters parameters = extractParameters(spec, metadata->uniqueName());
  return parameters->findFile(*(m_fileItemName.values(index).begin()));
}

std::string ResourceIOGroup::resourceForOperation(const Operation::Index& index) const
{
  auto vals = values(index);
  return !vals.empty() ? *vals.begin() : "";
}

std::set<Operation::Index> ResourceIOGroup::operationsForResource(
  const std::string& resourceName) const
{
  std::set<Operation::Index> operations;

  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return operations;
  }

  std::set<Operation::Index> allOperations = this->operations();

  for (auto& index : allOperations)
  {
    if (resourceForOperation(index) == resourceName)
    {
      operations.insert(index);
    }
  }
  return operations;
}
}
}

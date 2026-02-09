//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_JobSpecs_h
#define smtk_operation_JobSpecs_h

#include "smtk/CoreExports.h"

#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/project/Project.h"

#include "smtk/task/Task.h"

#include "smtk/common/Visit.h"

#include <filesystem>
#include <sstream>

namespace smtk
{
namespace operation
{

/// Add a JobSpec to the \a result.
///
/// This can be used to create a \a jobSpecType attribute of any type
/// that allows the objects in the \a associations container to be
/// associated with it.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addJobSpecWithAssociations(
  smtk::operation::Operation::Result result,
  const Container& associations,
  const std::string& jobSpecType = "JobSpec")
{
  if (!result)
  {
    return nullptr;
  }
  auto specification = result->attributeResource();
  auto jobSpec = specification->createAttribute(jobSpecType);
  if (!jobSpec)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not create JobSpec of type \"" << jobSpecType << "\".");
    return nullptr;
  }
  auto assocItem = jobSpec->associations();
  if (!assocItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Hint of type \"" << jobSpecType << "\" does not allow associations.");
    return nullptr;
  }
  bool ok = true;
  for (const auto& association : associations)
  {
    ok &= assocItem->appendValue(association);
    if (!ok)
    {
      break;
    }
  }
  ok &= result->findReference("jobsToSubmit")->appendValue(jobSpec);
  if (!ok)
  {
    specification->removeAttribute(jobSpec);
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Hint of type \"" << jobSpecType << "\" does not allow objects.");
    jobSpec = nullptr;
  }
  return jobSpec;
}

/// Visit job specifications in the \a result that inherit \a jobSpecType and invoke \a functor on them.
///
/// The functor will be passed the specification type, case directory, job location (local/remote),
/// queueing system (sh/pbs/slurm/…), launch utility (sh/mpiexec/aprun/srun/) and script/log pairs.
template<typename Functor>
SMTK_ALWAYS_EXPORT inline smtk::common::Visited visitJobSpecs(
  smtk::operation::Operation::Result result,
  const std::string& jobSpecType,
  Functor functor)
{
  auto jobsToSubmitItem = result->findReference("jobsToSubmit");
  if (!jobsToSubmitItem)
  {
    return smtk::common::Visited::Empty;
  }
  std::size_t numberOfJobSpecs = jobsToSubmitItem->numberOfValues();
  smtk::common::VisitorFunctor<Functor> ff(functor);
  bool didVisit = false;
  for (std::size_t ii = 0; ii < numberOfJobSpecs; ++ii)
  {
    auto jobToStart = jobsToSubmitItem->valueAs<smtk::attribute::Attribute>(ii);
    bool matchesType = false;
    if (!jobToStart)
    {
      continue;
    }
    for (const auto& attributeType : jobToStart->types())
    {
      if (attributeType == jobSpecType)
      {
        matchesType = true;
        break;
      }
    }
    if (!matchesType)
    {
      continue;
    }

    auto caseDirectory = jobToStart->findDirectory("case directory")->value();
    auto jobLocation = smtk::string::Token(jobToStart->findString("job location")->value());
    auto jobQueueing = smtk::string::Token(jobToStart->findString("job queueing system")->value());
    auto jobLauncher = smtk::string::Token(jobToStart->findString("job launcher")->value());
    auto scripts = jobToStart->findGroup("scripts");
    std::size_t numberOfScripts = scripts->numberOfGroups();
    std::vector<std::pair<std::filesystem::path, std::vector<std::filesystem::path>>> scriptLogs;
    scriptLogs.reserve(numberOfScripts);
    for (std::size_t jj = 0; jj < numberOfScripts; ++jj)
    {
      auto script =
        std::dynamic_pointer_cast<smtk::attribute::FileItem>(scripts->item(jj, 0))->value();
      auto logs = std::dynamic_pointer_cast<smtk::attribute::FileItem>(scripts->item(jj, 1))
                    ->valuesAs<std::filesystem::path>();
      scriptLogs.emplace_back(script, logs);
    }

    smtk::common::Visit action = ff(
      jobToStart->associations(), caseDirectory, jobLocation, jobQueueing, jobLauncher, scriptLogs);
    if (action == smtk::common::Visit::Halt)
    {
      return smtk::common::Visited::Some;
    }
    didVisit = true;
  }
  return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
}

} // namespace operation
} // namespace smtk

#endif // smtk_operation_JobSpecs_h
